#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/string.h>
#include <linux/time.h>
#define CTRL_REQUEST_TYPE	 0xc0
#define CTRL_REQUEST_STATUS      0x01
#define CTRL_REQUEST_RESCAN      0x02
#define CTRL_REQUEST_TEMP        0x03
#define CTRL_REQUEST_RESET       0x04
#define CTRL_VALUE               0x00
#define CTRL_INDEX	         0x00

#define PRODUCT_ID	0x05dc
#define VENDOR_ID	0x16c0

/* 设备驱动 注册设备 table of devices that work with this driver */
static const struct usb_device_id usbtemp_id_table[] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, usbtemp_id_table);



/* Structure to hold all of our device specific stuff */
struct usbtemp {
	struct usb_device	*udev;	/* usb  the usb device for this device */
	unsigned char *ctrl_in_buffer;	/*   the buffer to receive data */

	int supported_probes;
	__u8 int_in_endpointAddr;

 	//int temperature1;
	//int temperature2;
	int temperature[0];


};

/* give a dynamic allocation of memory */
static int get_status(struct usbtemp *temp_dev)
{
	int rc = 0;

	temp_dev->ctrl_in_buffer = kzalloc(0x08, GFP_KERNEL);


	rc = usb_control_msg(temp_dev->udev,
		usb_rcvctrlpipe(temp_dev->udev, 0),
		CTRL_REQUEST_STATUS,
		CTRL_REQUEST_TYPE,
		CTRL_VALUE,
		CTRL_INDEX,
		temp_dev->ctrl_in_buffer,
		0x08,
		10000);
		//int usb_control_msg(struct usb_device * dev, unsigned int pipe, __u8 request,
        // __u8 requesttype, __u16 value, __u16 index, void * data, __u16 size, int timeout);

	if (rc < 0) {
		pr_err("temp: control message failed (%d)", rc);
		return rc;
	}

//	temp_dev->supported_probes = temp_dev->ctrl_in_buffer[6] & 0xff;

	temp_dev->supported_probes = temp_dev->ctrl_in_buffer[6] & 0xff;

	kfree(temp_dev->ctrl_in_buffer);

	return rc;
}



static ssize_t ds1820tousb_status_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usbtemp *temp_dev = usb_get_intfdata(intf);

	get_status(temp_dev);

	return sprintf(buf, "supported probes: %d\n",
			temp_dev->supported_probes);

}


static SENSOR_DEVICE_ATTR(status, 0444, ds1820tousb_status_show, NULL, 0);

static int get_temp_value(struct usbtemp *temp_dev)
{
	int rc;
	int i;
	int sennum;
	temp_dev->ctrl_in_buffer = kzalloc(0x80, GFP_KERNEL);
	//sizeof(temp_dev->ctrl_in_buffer)/ 
	sennum = sizeof(temp_dev->ctrl_in_buffer)/sizeof(temp_dev->ctrl_in_buffer[0])/4;
	// sennum = sizeof(temp_dev->ctrl_in_buffer[0]); 
	rc =0;
	rc = usb_control_msg(temp_dev->udev,
		usb_rcvctrlpipe(temp_dev->udev, 0),
		CTRL_REQUEST_TEMP,
		CTRL_REQUEST_TYPE,
		CTRL_VALUE,
		CTRL_INDEX,
		temp_dev->ctrl_in_buffer,
		0x80,
		10000);

	if (rc < 0) {
		pr_err("temp: control message failed (%d)", rc);
		return rc;
	}

	for (i = 0 ;i < sennum;i++)
	{
		temp_dev->temperature[i] =
		(temp_dev->ctrl_in_buffer[8+16*i] & 0xff) +
		((temp_dev->ctrl_in_buffer[9+16*i] & 0xff) << 8);
	}

	// temp_dev->temperature2 =
	// 	(temp_dev->ctrl_in_buffer[24] & 0xff) +
	// 	((temp_dev->ctrl_in_buffer[25] & 0xff) << 8);





	kfree(temp_dev->ctrl_in_buffer);
	return rc;

}



static ssize_t ds1820tousb_temperature_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	int j;
	int k;
 	char A1[99];
	char A2[99];
	char K1[99];
 
	struct usb_interface *intf = to_usb_interface(dev);
	struct usbtemp *temp_dev = usb_get_intfdata(intf);

	// char   todayDateStr[100];
	// time_t rawtime;
	// struct tm *timeinfo;

	// time ( &rawtime );
	// timeinfo = localtime ( &rawtime );
	// strftime(todayDateStr, strlen("DD-MMM-YYYY HH:MM")+1,"%d-%b-%Y %H:%M",timeinfo);

    //printf("%s\n", s);
 	// p = gmtime(nowtime);

	k = sizeof(temp_dev->ctrl_in_buffer)/ sizeof(temp_dev->ctrl_in_buffer[0])/4;
	get_temp_value(temp_dev);


//sizeof(temp_dev->temperature)/sizeof(temp_dev->temperature[0])
	for (j=0;j<k; j++)
	{	
		
		int a1 = temp_dev->temperature[j]/16;
		int a2 =(temp_dev->temperature[j]*10000/16)%10000;
		sprintf(A1,"%d",a1);
		sprintf(A2,"%d",a2);
		sprintf(K1,"%d",k);
		// char dot = ".";
		// char c1 = "°C";
		strcat(A1,".");
		strcat(A1,A2);
		strcat(A1,"°C");
		strcat(A1,";");
		 // strcat(buf,K1);
		strcat(buf,A1);

	//	buf[j]= strcat( temp_dev->temperature[j]/16,".",(temp_dev->temperature[j]*10000/16)%10000,"°C"," ; ");
	 //   sprintf(buf, "%d%s%d%s%s\n", temp_dev->temperature[j]/16,".",(temp_dev->temperature[j]*10000/16)%10000,"°C"," ; ");
	}

	return sprintf(buf,"%s\n",  buf);

	
}
// chmod 
static SENSOR_DEVICE_ATTR(temp_input, 0444, ds1820tousb_temperature_show, NULL, 0);




// static ssize_t ds1820tousb_temperature2_show(struct device *dev,
// 						struct device_attribute *attr,
// 						char *buf)
// {
// 	struct usb_interface *intf = to_usb_interface(dev);
// 	struct usbtemp *temp_dev = usb_get_intfdata(intf);

// 	get_temp_value(temp_dev);

// 	return sprintf(buf, "%d%s%d%s\n", temp_dev->temperature2/16,".",(temp_dev->temperature2*1000/16)%1000,"°C");


// }

// static SENSOR_DEVICE_ATTR(temp2_input, 0444, ds1820tousb_temperature2_show, NULL, 0);




static int usb_rescan(struct usbtemp *temp_dev)
{
	int rc = 0;
	int an = 0;

	temp_dev->ctrl_in_buffer = kzalloc(0x08, GFP_KERNEL);

	rc = usb_control_msg(temp_dev->udev,
			usb_rcvctrlpipe(temp_dev->udev, 0),
			CTRL_REQUEST_RESCAN,
			CTRL_REQUEST_TYPE,
			CTRL_VALUE,
			CTRL_INDEX,
			temp_dev->ctrl_in_buffer,
			0x08,
			10000);

	if (rc < 0) {
		pr_err("temp: control message failed (%d)", rc);
		return rc;
	}

	pr_info("rescan");

	an = temp_dev->ctrl_in_buffer[0];

	if (an == 23)
		pr_info("New recognition is started");

	if (an == 42)
		pr_info("New recognition is already carried out");

	// neu anfragen
	//sysfs_create_file
	//sysfs.h  Attibut   


		// 	static inline void sysfs_remove_files(struct kobject *kobj,
		// 				     const struct attribute * const *attr)
		// {
		// }


		// static inline int sysfs_create_files(struct kobject *kobj,
		// 				    const struct attribute * const *attr)
		// {
		// 	return 0;
		// }





	kfree(temp_dev->ctrl_in_buffer);
	return rc;


}



static ssize_t ds1820tousb_rescan_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usbtemp *temp_dev = usb_get_intfdata(intf);
	unsigned long val;
	int ret;

	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	if (val == 1)
		usb_rescan(temp_dev);

	return count;


}

static SENSOR_DEVICE_ATTR(rescan, 0200, NULL, ds1820tousb_rescan_store, 0);




static int usb_reset(struct usbtemp *temp_dev)
{
	int rc = 0;


	temp_dev->ctrl_in_buffer = kzalloc(0x08, GFP_KERNEL);

	rc = usb_control_msg(temp_dev->udev,
			usb_rcvctrlpipe(temp_dev->udev, 0),
			CTRL_REQUEST_RESET,
			CTRL_REQUEST_TYPE,
			CTRL_VALUE,
			CTRL_INDEX,
			temp_dev->ctrl_in_buffer,
			0x08,
			10000);
	if (rc > 0) {
		pr_err("temp: reset failed\n");
		return rc;
	}

	pr_info("reset");

	kfree(temp_dev->ctrl_in_buffer);
	return rc;

}




static ssize_t ds1820tousb_reset_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct usbtemp *temp_dev = usb_get_intfdata(intf);
	unsigned long val;
	int ret;

	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	if (val == 1)
		usb_reset(temp_dev);

	return count;

}


static SENSOR_DEVICE_ATTR(reset, 0200, NULL, ds1820tousb_reset_store, 0);



// ds1820tousb_attrs[sensornummer+3]
static struct attribute *ds1820tousb_attrs[] = {
		&sensor_dev_attr_rescan.dev_attr.attr,
	&sensor_dev_attr_reset.dev_attr.attr,
	&sensor_dev_attr_status.dev_attr.attr,


	&sensor_dev_attr_temp_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	// &sensor_dev_attr_temp3_input.dev_attr.attr,
	// &sensor_dev_attr_temp4_input.dev_attr.attr,
	// &sensor_dev_attr_temp5_input.dev_attr.attr,
	// &sensor_dev_attr_temp6_input.dev_attr.attr,
	// &sensor_dev_attr_temp7_input.dev_attr.attr,

	// &sensor_dev_attr_temp2_input.dev_attr.attr,

	NULL
};

ATTRIBUTE_GROUPS(ds1820tousb);



static int usbtemp_probe(struct usb_interface *interface,
			const struct usb_device_id *id)
{
	struct usbtemp *dev;
	struct device *hwmon_dev;
	int retval;
	int i;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint = NULL;


	/* allocate memory for our device state and initialize it */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->udev = usb_get_dev(interface_to_usbdev(interface));


	/* find the one control endpoint of this device */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (usb_endpoint_is_int_in(endpoint)) {
			dev->int_in_endpointAddr = endpoint->bEndpointAddress;
			break;
		}
	}
	if (!dev->int_in_endpointAddr) {
		dev_err(&interface->dev, "Could not find int-in endpoint");
		retval = -ENODEV;
		goto error;
	}


	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);


	hwmon_dev = hwmon_device_register_with_groups(&interface->dev,
							dev->udev->product,
							dev, ds1820tousb_groups);
	return 0;
error:
	usb_set_intfdata(interface, NULL);
	kfree(dev->ctrl_in_buffer);
	kfree(dev);
	return retval;
}


static void usbtemp_disconnect(struct usb_interface *interface)
{
	struct usbtemp *dev;

	dev = usb_get_intfdata(interface);

	hwmon_device_unregister((&interface->dev));

	usb_set_intfdata(interface, NULL);

	usb_put_dev(dev->udev);

	kfree(dev);

	dev_info(&interface->dev, "USB Temp now disconnected\n");
}



static struct usb_driver usbtemp_driver = {
	.name =		"usbtemp",
	.probe =	usbtemp_probe,
	.disconnect =	usbtemp_disconnect,
	.id_table =	usbtemp_id_table
};



static int __init usbtemp_init(void)
{
	pr_info("usbtemp initialized \n");
	return usb_register(&usbtemp_driver);
}

static void __exit usbtemp_exit(void)
{
	usb_deregister(&usbtemp_driver);
}

module_init(usbtemp_init);
module_exit(usbtemp_exit);


MODULE_AUTHOR("Yuhan Lin");
MODULE_DESCRIPTION("ds1820tousb driver");
MODULE_LICENSE("GPL");
