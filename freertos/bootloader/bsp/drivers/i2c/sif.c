#include <sif.h>
#include <errno.h>

static void _sif_configSifSpeed(const unsigned int base_address, const unsigned int clk_spd)
{
    unsigned int data = (clk_spd << SIF_CLK_DIV_BIT) | (SIF_EN_MSK << SIF_EN_BIT);
    unsigned int mask = SIF_CLK_DIV_SM | SIF_EN_SM;
    SIF_WRITE_MASK(base_address, RG_SIF_CTRL0, data, mask);
}

static void _sif_reverseAddr(unsigned int *ptr_addr)
{
    *ptr_addr = (*ptr_addr & 0xFFFF0000) >> 16 | (*ptr_addr & 0x0000FFFF) << 16;
    *ptr_addr = (*ptr_addr & 0xFF00FF00) >> 8  | (*ptr_addr & 0x00FF00FF) << 8;
}

static int _sif_checkBus(const unsigned int base_address)
{
    unsigned int reg_value;
    reg_value = SIF_READ(base_address, RG_SIF_CTRL0);
    return ((reg_value & (SIF_SDA_STATE_SM)) && (reg_value & (SIF_SCL_STATE_SM))) ? 0 : EBUSY;
}

static int _sif_putModeBit(const unsigned int base_address, SIF_MODE_T op_mode, unsigned int page_len)
{
    unsigned int loopcount = 0;
    unsigned int data = (page_len << SIF_PGLEN_BIT) | (op_mode << SIF_MODE_BIT) | SIF_TRI_MSK;
    unsigned int mask = SIF_PGLEN_SM | SIF_MODE_SM | SIF_TRI_SM;

    SIF_WRITE_MASK(base_address, RG_SIF_CTRL1, data, mask);

    while (data & SIF_TRI_MSK)
    {
        data = SIF_READ(base_address, RG_SIF_CTRL1);

        if (++loopcount >= 0xffff)
        {
            return ETIME;
        }
    }
    return 0;
}

#define _sif_putStart(__ADDR__)                 _sif_putModeBit(__ADDR__, SIF_START, 0)
#define _sif_putWrite(__ADDR__, __LEN__)        _sif_putModeBit(__ADDR__, SIF_WRITE_DATA, __LEN__)
#define _sif_putStop(__ADDR__)                  _sif_putModeBit(__ADDR__, SIF_STOP, 0)
#define _sif_putReadFinal(__ADDR__, __LEN__)    _sif_putModeBit(__ADDR__, SIF_READ_FINAL, __LEN__)

static int _sif_checkAck(const unsigned int base_address, const unsigned int expect_ack)
{
    unsigned int reg_value = 0, ack = 0;

    reg_value = SIF_READ(base_address, RG_SIF_CTRL1);
    reg_value &= SIF_ACK_SM;

    while (reg_value)
    {
        reg_value &= (reg_value - 1);
        ack++;
    }
    return (ack == expect_ack) ? (1) : (0);
}

static int _sif_putSlaveId(const unsigned int base_address, const unsigned int slave_id)
{
    int rc = 0;

    unsigned int data = slave_id;
    unsigned int mask = 0xFF;

    SIF_WRITE_MASK(base_address, RG_SIF_DATA0, data, mask);

    rc = _sif_putWrite(base_address, 0);
    if (0 != rc)
    {
        return rc;
    }

    if (0 == _sif_checkAck(base_address, 1))
    {
        return ENODATA;
    }
    return rc;
}

static int _sif_putMultiBytes(unsigned int base_address, unsigned int *ptr_data)
{
    int rc = 0;

    /* write to data register */
    SIF_WRITE(base_address, RG_SIF_DATA0, *ptr_data);

    if (0 == _sif_putWrite(base_address, SIF_DATA_LEN - 1))
    {
        if (0 == _sif_checkAck(base_address, SIF_DATA_LEN))
        {
            rc = ENODATA;
        }
    }
    return rc;
}

#define _sif_putAddress(__ADDR__, __PTR_DATA__) _sif_putMultiBytes(__ADDR__, __PTR_DATA__)

static int _sif_getMultiBytes(const unsigned int base_address, unsigned int *ptr_data)
{
    int rc = 0;
    rc = _sif_putReadFinal(base_address, SIF_DATA_LEN - 1);
    if(0 == rc)
    {
        *ptr_data = SIF_READ(base_address, RG_SIF_DATA0);
    }
    return rc;
}

static int
_sif_rwOperation(
    const SIF_OPERATION_T  rw_operation,
    SIF_PARAM_T            *ptr_sif_param
)
{
    int rc = 0;
    int base_address = (ptr_sif_param->bus == 0) ? (RG_SIF0_BASE_ADDR) : (RG_SIF1_BASE_ADDR);

    rc = _sif_checkBus(base_address);
    if (0 != rc)
    {
        if (0 == _sif_putStop(base_address))
        {
            rc = _sif_checkBus(base_address);
        }
    }

    if (0 == rc)
    {
        if (0 == _sif_putStart(base_address))
        {
            rc = _sif_putSlaveId(base_address, ((ptr_sif_param->slave_id << 1) | 0));

            if (0 == rc)
            {
                _sif_reverseAddr(&ptr_sif_param->addr);
                rc = _sif_putAddress(base_address, &ptr_sif_param->addr);
            }
        }
    }

    if (0 == rc)
    {
        if (rw_operation)
        {
            if (0 == _sif_putStart(base_address))
            {
                rc = _sif_putSlaveId(base_address, (ptr_sif_param->slave_id << 1 | 1));

                if (0 == rc)
                {
                    rc = _sif_getMultiBytes(base_address, ptr_sif_param->ptr_data);
                }
            }
        }
        else
        {
            rc = _sif_putMultiBytes(base_address, ptr_sif_param->ptr_data);
        }
        rc = _sif_putStop(base_address);
    }
    return rc;
}

void sif_init(void)
{
    _sif_configSifSpeed(RG_SIF1_BASE_ADDR, SIF_SPEED_1M);
}

int sif_write(SIF_PARAM_T  *ptr_sif_param)
{
    return _sif_rwOperation(SIF_WRITE, ptr_sif_param);
}

int sif_read(SIF_PARAM_T  *ptr_sif_param)
{
    return _sif_rwOperation(SIF_READ, ptr_sif_param);
}