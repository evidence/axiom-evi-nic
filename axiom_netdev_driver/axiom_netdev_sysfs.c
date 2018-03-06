/*!
 * \file axiom_netdev_sysfs.c
 *
 * \version     v1.1
 * \date        2017-12-21
 *
 * This file contains the implementation of the AXIOM sysfs.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include <axiom_netdev.h>

static inline ssize_t
axsys_uint8_show(char *buf, uint8_t value)
{
    return snprintf(buf, PAGE_SIZE, "%hhu\n", value);
}

static inline ssize_t
axsys_uint8_store(const char *buf, size_t count, uint8_t *value)
{
    int ret;

    ret = kstrtou8(buf, 0, value);
    if (ret)
        return -EINVAL;

    return count;
}

static inline ssize_t
axsys_int32_show(char *buf, int32_t value)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

static inline ssize_t
axsys_int32_store(const char *buf, size_t count, int32_t *value)
{
    int ret;

    ret = kstrtos32(buf, 0, value);
    if (ret)
        return -EINVAL;

    return count;
}

static inline ssize_t
axsys_uint32_show(char *buf, uint32_t value)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static inline ssize_t
axsys_uint32_store(const char *buf, size_t count, uint32_t *value)
{
    int ret;

    ret = kstrtou32(buf, 0, value);
    if (ret)
        return -EINVAL;

    return count;
}

static inline ssize_t
axsys_uint64_show(char *buf, uint64_t value)
{
    return snprintf(buf, PAGE_SIZE, "%llu\n", value);
}

static inline ssize_t
axsys_uint64_store(const char *buf, size_t count, uint64_t *value)
{
    int ret;

    ret = kstrtou64(buf, 0, value);
    if (ret)
        return -EINVAL;

    return count;
}

/* watchdog_period_msec callbacks */
static ssize_t
axsys_watchdog_period_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));

    return axsys_uint32_show(buf, axsys->watchdog_period_msec);
}
static ssize_t
axsys_watchdog_period_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));
    ssize_t ret;

    ret = axsys_uint32_store(buf, count, &axsys->watchdog_period_msec);

    /* wakeup kthread */
    axiom_kthread_wakeup(&axsys->drvdata->kthread_wtd);

    return ret;
}
static DEVICE_ATTR(watchdog_period_msec, S_IRUGO | S_IWUSR,
        axsys_watchdog_period_show, axsys_watchdog_period_store);

/* retry_delay_usec callbacks */
static ssize_t
axsys_retry_delay_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));

    return axsys_uint32_show(buf, axsys->retry_delay_usec);
}
static ssize_t
axsys_retry_delay_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));

    return axsys_uint32_store(buf, count, &axsys->retry_delay_usec);
}
static DEVICE_ATTR(retry_delay_usec, S_IRUGO | S_IWUSR,
        axsys_retry_delay_show, axsys_retry_delay_store);

static struct attribute *axiom_sysfs_param_attrs[] = {
    &dev_attr_watchdog_period_msec.attr,
    &dev_attr_retry_delay_usec.attr,
    NULL
};
ATTRIBUTE_GROUPS(axiom_sysfs_param);

static ssize_t
axsys_nodeid_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));

    return axsys_uint8_show(buf, axsys->drvdata->node_id);
}
static DEVICE_ATTR(nodeid, S_IRUGO, axsys_nodeid_show, NULL);

static ssize_t
axsys_ifnumber_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));
    uint8_t ifnumber;

    if (axiom_hw_get_if_number(axsys->drvdata->dev_api, &ifnumber))
        return -EFAULT;

    return axsys_uint8_show(buf, ifnumber);
}
static DEVICE_ATTR(ifnumber, S_IRUGO, axsys_ifnumber_show, NULL);

static struct attribute *axiom_sysfs_info_attrs[] = {
    &dev_attr_nodeid.attr,
    &dev_attr_ifnumber.attr,
    NULL
};
ATTRIBUTE_GROUPS(axiom_sysfs_info);

static ssize_t
axsys_pid_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
    struct kobject *kobj = (struct kobject *)dev;
    struct axiomnet_sysfs *axsys = dev_get_drvdata(kobj_to_dev(kobj->parent));
    struct axiom_kthread *kthread = NULL;

    if (kobj == axsys->kthread_raw) {
        kthread = &axsys->drvdata->kthread_raw;
    } else if (kobj == axsys->kthread_rdma) {
        kthread = &axsys->drvdata->kthread_rdma;
    } else if (kobj == axsys->kthread_wtd) {
        kthread = &axsys->drvdata->kthread_wtd;
    } else {
        return -EFAULT;
    }

    return axsys_uint32_show(buf, axiom_kthread_getpid(kthread));
}
static DEVICE_ATTR(pid, S_IRUGO, axsys_pid_show, NULL);

static struct attribute *axiom_sysfs_kthread_attrs[] = {
    &dev_attr_pid.attr,
    NULL
};
ATTRIBUTE_GROUPS(axiom_sysfs_kthread);

int
axiom_sysfs_init(const char *name, struct axiomnet_sysfs *axsys,
        struct axiomnet_drvdata *drvdata)
{
    struct kobject *root;
    int ret = 0;

    axsys->dev = root_device_register(name);
    if (IS_ERR(axsys->dev)) {
        EPRINTF("Unable to register %s root device", name);
        return PTR_ERR(axsys->dev);
    }
    root = &axsys->dev->kobj;

    axsys->param = kobject_create_and_add("parameters", root);
    if (!axsys->param) {
        goto free_root;
    }

    ret = sysfs_create_groups(axsys->param, axiom_sysfs_param_groups);
    if (ret) {
        EPRINTF("Unable to create group of param attributes");
        goto free_paramk;
    }

    axsys->info = kobject_create_and_add("info", root);
    if (!axsys->info) {
        goto free_paramg;
    }

    ret = sysfs_create_groups(axsys->info, axiom_sysfs_info_groups);
    if (ret) {
        EPRINTF("Unable to create group of info attributes");
        goto free_infok;
    }

    axsys->kthread_raw = kobject_create_and_add("kthread_raw", root);
    if (!axsys->kthread_raw) {
        goto free_infog;
    }

    ret = sysfs_create_groups(axsys->kthread_raw,
            axiom_sysfs_kthread_groups);
    if (ret) {
        EPRINTF("Unable to create group of kthread_raw attributes");
        goto free_sched_rawk;
    }

    axsys->kthread_rdma = kobject_create_and_add("kthread_rdma", root);
    if (!axsys->kthread_rdma) {
        goto free_sched_rawg;
    }

    ret = sysfs_create_groups(axsys->kthread_rdma,
            axiom_sysfs_kthread_groups);
    if (ret) {
        EPRINTF("Unable to create group of kthread_rdma attributes");
        goto free_sched_rdmak;
    }

    axsys->kthread_wtd = kobject_create_and_add("kthread_wtd", root);
    if (!axsys->kthread_wtd) {
        goto free_sched_rdmag;
    }

    ret = sysfs_create_groups(axsys->kthread_wtd,
            axiom_sysfs_kthread_groups);
    if (ret) {
        EPRINTF("Unable to create group of kthread_wtd attributes");
        goto free_sched_wtdk;
    }

    dev_set_drvdata(axsys->dev, axsys);
    axsys->drvdata = drvdata;

    return 0;

free_sched_wtdk:
    kobject_put(axsys->kthread_wtd);
free_sched_rdmag:
    sysfs_remove_groups(root, axiom_sysfs_kthread_groups);
free_sched_rdmak:
    kobject_put(axsys->kthread_rdma);
free_sched_rawg:
    sysfs_remove_groups(root, axiom_sysfs_kthread_groups);
free_sched_rawk:
    kobject_put(axsys->kthread_raw);
free_infog:
    sysfs_remove_groups(root, axiom_sysfs_info_groups);
free_infok:
    kobject_put(axsys->info);
free_paramg:
    sysfs_remove_groups(root, axiom_sysfs_param_groups);
free_paramk:
    kobject_put(axsys->param);
free_root:
    root_device_unregister(axsys->dev);
    axsys->dev = NULL;

    return ret;
}

void
axiom_sysfs_uninit(struct axiomnet_sysfs *axsys)
{
    if (!axsys->dev)
        return;

    sysfs_remove_groups(axsys->kthread_wtd, axiom_sysfs_kthread_groups);
    kobject_put(axsys->kthread_wtd);
    sysfs_remove_groups(axsys->kthread_rdma, axiom_sysfs_kthread_groups);
    kobject_put(axsys->kthread_rdma);
    sysfs_remove_groups(axsys->kthread_raw, axiom_sysfs_kthread_groups);
    kobject_put(axsys->kthread_raw);
    sysfs_remove_groups(axsys->info, axiom_sysfs_info_groups);
    kobject_put(axsys->info);
    sysfs_remove_groups(axsys->param, axiom_sysfs_param_groups);
    kobject_put(axsys->param);
    root_device_unregister(axsys->dev);
    axsys->dev = NULL;
}
