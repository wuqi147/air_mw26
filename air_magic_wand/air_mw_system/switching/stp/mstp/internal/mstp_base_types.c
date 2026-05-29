
// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

#include "mstp_base_types.h"
#include "mstp_log.h"

// ============================================================================

const char* GetPortRoleName(STP_PORT_ROLE role)
{
    if (role == STP_PORT_ROLE_MASTER)
        return "Master";
    else if (role == STP_PORT_ROLE_ROOT)
        return "Root";
    else if (role == STP_PORT_ROLE_DESIGNATED)
        return "Designated";
    else if (role == STP_PORT_ROLE_ALTERNATE)
        return "Alternate";
    else if (role == STP_PORT_ROLE_BACKUP)
        return "Backup";
    else if (role == STP_PORT_ROLE_DISABLED)
        return "Disabled";
    else
    {
        return "undefined";
    }
}
