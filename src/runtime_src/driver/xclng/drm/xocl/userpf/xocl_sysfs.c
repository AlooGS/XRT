/*
 * A GEM style device manager for PCIe based OpenCL accelerators.
 *
 * Copyright (C) 2016-2018 Xilinx, Inc. All rights reserved.
 *
 * Authors: Lizhi.Hou@xilinx.com
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "common.h"

//Attributes followed by bin_attributes.
//
/* -Attributes -- */

/* -xclbinuuid-- (supersedes xclbinid) */
static ssize_t xclbinuuid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct xocl_dev *xdev = dev_get_drvdata(dev);
	return sprintf(buf, "%pUb\n", &xdev->xclbin_id);
}

static DEVICE_ATTR_RO(xclbinuuid);

/* -userbar-- */
static ssize_t userbar_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct xocl_dev *xdev = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", xdev->core.priv.user_bar);
}

static DEVICE_ATTR_RO(userbar);

static ssize_t user_pf_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	// The existence of entry indicates user function.
	return sprintf(buf, "%s", "");
}
static DEVICE_ATTR_RO(user_pf);

/* -live client contects-- */
static ssize_t kdsstat_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct xocl_dev *xdev = dev_get_drvdata(dev);
	int size = sprintf(buf,
			   "xclbin:\t\t\t%pUb\noutstanding execs:\t%d\ntotal execs:\t\t%ld\ncontexts:\t\t%d\n",
			   &xdev->xclbin_id,
			   atomic_read(&xdev->outstanding_execs),
			   atomic64_read(&xdev->total_execs),
			   get_live_client_size(xdev));
	buf += size;
	if (xdev->layout == NULL)
		return size;
#if 0
	// Enable in 2019.1
	for (i = 0; i < xdev->layout->m_count; i++) {
		if (xdev->layout->m_ip_data[i].m_type != IP_KERNEL)
			continue;
		size += sprintf(buf, "\t%s:\t%d\n", xdev->layout->m_ip_data[i].m_name,
				xdev->ip_reference[i]);
		buf += size;
	}
#endif
	return size;
}
static DEVICE_ATTR_RO(kdsstat);

/* -live memory usage-- */
static ssize_t memstat_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct xocl_dev *xdev = dev_get_drvdata(dev);
	return xocl_mm_sysfs_stat(xdev, buf, false);
}
static DEVICE_ATTR_RO(memstat);

static ssize_t memstat_raw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct xocl_dev *xdev = dev_get_drvdata(dev);
	return xocl_mm_sysfs_stat(xdev, buf, true);
}
static DEVICE_ATTR_RO(memstat_raw);

/* - End attributes-- */

/* - Begin bin_attributes -- */

//- Debug IP_layout--
static ssize_t read_debug_ip_layout(struct file *filp, struct kobject *kobj,
	struct bin_attribute *attr, char *buffer, loff_t offset, size_t count)
{
	struct xocl_dev *xdev;
	u32 nread = 0;
	size_t size = 0;

	xdev = dev_get_drvdata(container_of(kobj, struct device, kobj));

	size = sizeof_sect(xdev->debug_layout, m_debug_ip_data);
	if (offset >= size)
		return 0;

	if (count < size - offset)
		nread = count;
	else
		nread = size - offset;

	memcpy(buffer, ((char *)xdev->debug_layout) + offset, nread);

	return nread;
}
static struct bin_attribute debug_ip_layout_attr = {
	.attr = {
		.name = "debug_ip_layout",
		.mode = 0444
	},
	.read = read_debug_ip_layout,
	.write = NULL,
	.size = 0
};


//IP layout
static ssize_t read_ip_layout(struct file *filp, struct kobject *kobj,
	struct bin_attribute *attr, char *buffer, loff_t offset, size_t count)
{
	const struct xocl_dev *xdev;
	u32 nread = 0;
	size_t size = 0;

	xdev = dev_get_drvdata(container_of(kobj, struct device, kobj));

	size = sizeof_sect(xdev->layout, m_ip_data);
	if (offset >= size)
		return 0;

	if (count < size - offset)
		nread = count;
	else
		nread = size - offset;

	memcpy(buffer, ((char *)xdev->layout) + offset, nread);

	return nread;
}

static struct bin_attribute ip_layout_attr = {
	.attr = {
		.name = "ip_layout",
		.mode = 0444
	},
	.read = read_ip_layout,
	.write = NULL,
	.size = 0
};

//-Connectivity--
static ssize_t read_connectivity(struct file *filp, struct kobject *kobj,
	struct bin_attribute *attr, char *buffer, loff_t offset, size_t count)
{
	struct xocl_dev *xdev;
	u32 nread = 0;
	size_t size = 0;

	xdev = dev_get_drvdata(container_of(kobj, struct device, kobj));

	size = sizeof_sect(xdev->connectivity, m_connection);
	if (offset >= size)
		return 0;

	if (count < size - offset)
		nread = count;
	else
		nread = size - offset;

	memcpy(buffer, ((char *)xdev->connectivity) + offset, nread);

	return nread;

}

static struct bin_attribute connectivity_attr = {
	.attr = {
		.name = "connectivity",
		.mode = 0444
	},
	.read = read_connectivity,
	.write = NULL,
	.size = 0
};

//-Mem_topology--
static ssize_t read_mem_topology(struct file *filp, struct kobject *kobj,
	struct bin_attribute *attr, char *buffer, loff_t offset, size_t count)
{
	struct xocl_dev *xdev;
	u32 nread = 0;
	size_t size = 0;

	xdev = dev_get_drvdata(container_of(kobj, struct device, kobj));

	size = sizeof_sect(xdev->topology, m_mem_data);
	if (offset >= size)
		return 0;

	if (count < size - offset)
		nread = count;
	else
		nread = size - offset;

	memcpy(buffer, ((char *)xdev->topology) + offset, nread);

	return nread;
}


static struct bin_attribute mem_topology_attr = {
	.attr = {
		.name = "mem_topology",
		.mode = 0444
	},
	.read = read_mem_topology,
	.write = NULL,
	.size = 0
};

static ssize_t reset_store(struct device *dev, struct device_attribute *da,
	const char *buf, size_t count)
{
	int ret = xocl_hot_reset(dev_get_drvdata(dev), false);

	if (ret == 0)
		return count;

	return ret;
}
static DEVICE_ATTR_WO(reset);

static struct attribute *xocl_attrs[] = {
	&dev_attr_xclbinuuid.attr,
	&dev_attr_userbar.attr,
	&dev_attr_kdsstat.attr,
	&dev_attr_memstat.attr,
	&dev_attr_memstat_raw.attr,
	&dev_attr_user_pf.attr,
	&dev_attr_reset.attr,
	NULL,
};

static struct bin_attribute *xocl_bin_attrs[] = {
	&debug_ip_layout_attr,
	&ip_layout_attr,
	&connectivity_attr,
	&mem_topology_attr,
	NULL,
};

static struct attribute_group xocl_attr_group = {
	.attrs = xocl_attrs,
	.bin_attrs = xocl_bin_attrs,
};

//---
int xocl_init_sysfs(struct device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->kobj, &xocl_attr_group);
	if (ret)
		xocl_err(dev, "create xocl attrs failed: %d", ret);

	return ret;
}

void xocl_fini_sysfs(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &xocl_attr_group);
}