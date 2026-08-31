/******************************************************************************
**
** FILE NAME    : ifxmips_pcie_pm.h
** PROJECT      : IFX UEIP
** MODULES      : PCIe Root Complex Driver
**
** DATE         : 21 Dec 2009
** AUTHOR       : Lei Chuanhua
** DESCRIPTION  : PCIe Root Complex Driver Power Managment
** COPYRIGHT    :       Copyright (c) 2009
**                      Lantiq Deutschland GmbH
**                      Am Campeon 3, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 21 Dec,2009   Lei Chuanhua    First UEIP release
*******************************************************************************/
/*!
 \file ifxmips_pcie_pm.h
 \ingroup IFX_PCIE
 \brief header file for PCIe Root Complex Driver Power Management
*/

#ifndef IFXMIPS_PCIE_PM_H
#define IFXMIPS_PCIE_PM_H

void ifx_pcie_pmcu_init(void);
void ifx_pcie_pmcu_exit(void);

#endif /* IFXMIPS_PCIE_PM_H  */

