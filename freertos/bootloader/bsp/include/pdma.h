
void pdma_test_tx(int burst);
void dump_pkt_data(unsigned char *addr, unsigned long len);
void pdma_dump_rx_desc(void);
void pdma_dump_tx_desc(void);
void pdma_debug(const char *cmd);
void pdma_rx_pkt(int rxq);
void pdma_performance_test(void);
int pdma_tx_pkt(unsigned char *data, int len, int txq, int cp_to_sec_desc);
void pdma_rx_ring_proc(int rxq);
void pdma_isr (void);
void pdma_init (void);
