/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 voltage request.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#include "ftv310_vpu_voltage.h"

int ftv310_vpu_acpi_voltage_request(struct device *dev, bool is_decode, unsigned long *volt)
{
	acpi_handle handle = ACPI_HANDLE(dev);

	union acpi_object args[3];
	struct acpi_object_list arg_list = {
			.pointer = args,
			.count = ARRAY_SIZE(args),
	};
	acpi_status status;
	unsigned long long ret;

	args[0].type = ACPI_TYPE_INTEGER;
	if (is_decode)
		args[0].integer.value = 3;/* decode */
	else
		args[0].integer.value = 4; /* encode */
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = 0;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = 0;


	if (!has_acpi_companion(dev))
		pr_err("get acpi device failed\n");

	status = acpi_evaluate_integer(handle, "PGDV", &arg_list, &ret);
	if (ACPI_FAILURE(status)) {
		pr_err("No PGDV Method\n");
		return -1;
	}
	if (ret < 0) {
		pr_err("Failed to get voltage");
		return -1;
	}

	*volt = (unsigned long)ret;
	return 0;
}

int ftv310_vpu_voltage_request(struct device *dev, bool is_decode, unsigned long *volt)
{
	int ret = 0;

	if (has_acpi_companion(dev))
		ret = ftv310_vpu_acpi_voltage_request(dev, is_decode, volt);

	return ret;
}

