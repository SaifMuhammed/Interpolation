/*  interpolation.c - The simplest kernel module.

* Copyright (C) 2013 - 2016 Xilinx, Inc
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program. If not, see <http://www.gnu.org/licenses/>.

*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include<linux/fs.h>
#include<linux/string.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

                   /* Standard module information, edit as appropriate */

MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("Xilinx Inc.");
MODULE_DESCRIPTION
    ("interpolation - loadable module template generated by petalinux-create -t modules");

#define DRIVER_NAME "interpolation"

                          /* Simple example of how to receive command line parameters to your module.
                            Delete if you don't need them */

#define CLASS_NAME "CUBIC_INTER"

static int ker_buf[2000];
static int input_1 =0;
/*static int input_2 =0;
static int input_3 =0;
static int input_4 =0;
static int input_5 =0;
static int input_6 =0;
static int input_7 =0;
static int input_8 =0;
*/

static int result =0;
static int resp_ready =0,wr_ready=0;
static struct class *driver_class=NULL;
static dev_t first;
static struct cdev c_dev;
static struct device *ourDevice;
volatile unsigned int *x1;
volatile unsigned int *x2;
volatile unsigned int *x3;
volatile unsigned int *x4;
volatile unsigned int *f1;
volatile unsigned int *f2;
volatile unsigned int *f3;
volatile unsigned int *f4;
volatile unsigned int *output1;
volatile unsigned int *regCrtl;

struct interpolation_local {
	int irq;
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

                     //character call back

static int dev_open(struct inode *inod,struct file *fil);
static ssize_t dev_read(struct file *filep,char *buf,size_t len,loff_t *off);
static ssize_t dev_write(struct file *filep,char *buf,size_t len,loff_t *off);
static int dev_release(struct inode *inod ,struct file *fil);

void ipStart(void){
   unsigned int data =(*regCrtl & 0x80);
   *regCrtl=(data | 0x01);
}
unsigned int ipIsDone(void){
   unsigned int data = *regCrtl;
   return ((data >> 1) & 0x1);
} 


static struct file_operations f_ops={
             .read=dev_read,
             .write=dev_write,
             .open=dev_open,
             .release=dev_release,
}; 

static irqreturn_t interpolation_irq(int irq, void *lp){
	printk("interpolation interrupt\n");
	return IRQ_HANDLED;
}

static int interpolation_probe(struct platform_device *pdev){
	struct resource *r_irq; /* Interrupt resources */
	struct resource *r_mem; /* IO mem resources */
	struct device *dev = &pdev->dev;
	struct interpolation_local *lp = NULL;

	int rc = 0;
	dev_info(dev, "Device Tree Probing\n");

               	/* Get iospace for the device */

 	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(dev, "invalid address\n");
		return -ENODEV;
	}
	lp = (struct interpolation_local *) kmalloc(sizeof(struct interpolation_local), GFP_KERNEL);
	if (!lp) {
		dev_err(dev, "Cound not allocate interpolation device\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, lp);
	lp->mem_start = r_mem->start;
	lp->mem_end = r_mem->end;

	if (!request_mem_region(lp->mem_start,
				lp->mem_end - lp->mem_start + 1,
				DRIVER_NAME)) {
		dev_err(dev, "Couldn't lock memory region at %p\n",
			(void *)lp->mem_start);
		rc = -EBUSY;
		goto error1;
	}

	lp->base_addr = ioremap(lp->mem_start, lp->mem_end - lp->mem_start + 1);
	if (!lp->base_addr) {
		dev_err(dev, "interpolation: Could not allocate iomem\n");
		rc = -EIO;
		goto error2;
	}


                          //normal device driver

        if(alloc_chrdev_region(&first,0,1,"Leonardo") < 0){
           printk(KERN_ALERT "alloc chrdev region failed\n");
           return -1;
        }

                          //Create class(/sysfs)

         driver_class=class_create(THIS_MODULE,CLASS_NAME);
         if(driver_class == NULL){
         printk(KERN_ALERT "Create class failed\n");
         class_destroy(driver_class);
         unregister_chrdev_region(first,1);

         return -1;
         }


         if(device_create(driver_class,NULL,first,NULL,"interpolation") == NULL){
         printk(KERN_ALERT "Create device failed\n");
         device_destroy(driver_class,first);
         class_destroy(driver_class);
         unregister_chrdev_region(first,1);
         return -1;
         }
         
         cdev_init(&c_dev,&f_ops);
         if(cdev_add(&c_dev,first,1)==-1){
            printk(KERN_ALERT "Create character device failed\n");
            device_destroy(driver_class,first);
            class_destroy(driver_class);
            unregister_chrdev_region(first,1);
            return -1;
         }



	/* Get IRQ for the device */

 	r_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!r_irq) {
		dev_info(dev, "no IRQ found\n");
		dev_info(dev, "interpolation at 0x%08x mapped to 0x%08x\n",
			(unsigned int __force)lp->mem_start,(unsigned int __force)lp->base_addr);
                x1=(unsigned int __force)lp->base_addr + 0x10;
                //x2=(unsigned int __force)lp->base_addr + 0x14;
                //x3=(unsigned int __force)lp->base_addr + 0x18;
                //x4=(unsigned int __force)lp->base_addr + 0x1c;
               // f1=(unsigned int __force)lp->base_addr + 0x20;
             //   f2=(unsigned int __force)lp->base_addr + 0x24;
               // f3=(unsigned int __force)lp->base_addr + 0x28;
                //f4=(unsigned int __force)lp->base_addr + 0x2c;
                output1 =(unsigned int __force)lp->base_addr + 0x80; 
                regCrtl=(unsigned int __force)lp->base_addr + 0x00;
                printk("x1:0x%08x\n",(unsigned int)x1);
              /*  printk("x2:0x%08x\n",(unsigned int)x2);
                printk("x3:0x%08x\n",(unsigned int)x3);
                printk("x4:0x%08x\n",(unsigned int)x4);
                printk("f1:0x%08x\n",(unsigned int)f1);
                printk("f2:0x%08x\n",(unsigned int)f2);
                printk("f3:0x%08x\n",(unsigned int)f3);
                printk("f4:0x%08x\n",(unsigned int)f4);*/
                printk("output:0x%08x\n",(unsigned int)output1);
                printk("regCrtl:0x%08x\n",(unsigned int)regCrtl);
                wr_ready=1;
                return 0;
        }


	dev_info(dev,"interpolation at 0x%08x mapped to 0x%08x, irq=%d\n",
		(unsigned int __force)lp->mem_start,
		(unsigned int __force)lp->base_addr,
		lp->irq);
	return 0;
error2:
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
error1:
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return rc;
}

static int interpolation_remove(struct platform_device *pdev){
        struct device *dev = &pdev->dev;
        struct interpolation_local *lp = dev_get_drvdata(dev);
        free_irq(lp->irq, lp);
        release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
        kfree(lp);
        dev_set_drvdata(dev, NULL);
        return 0;
}

#ifdef CONFIG_OF
static struct of_device_id interpolation_of_match[] = {
        { .compatible = "xlnx,cubicSplineInterpolation-3.0", },
        { /* end of list */ },
};
MODULE_DEVICE_TABLE(of, interpolation_of_match);
#else
# define interpolation_of_match
#endif


static struct platform_driver interpolation_driver = {
        .driver = {
        	.name = DRIVER_NAME,
        	.owner = THIS_MODULE,
             	.of_match_table	= interpolation_of_match,
        },
          	.probe		= interpolation_probe,
          	.remove		= interpolation_remove,
};

static int __init interpolation_init(void){
        printk("<1> module world.\n");
        return platform_driver_register(&interpolation_driver);
}


static void __exit interpolation_exit(void){
        platform_driver_unregister(&interpolation_driver);
        printk(KERN_ALERT "Goodbye module world.\n");
}

static int dev_open(struct inode *inod,struct file *fil){
        printk(KERN_INFO  "device opened\n");
        return 0;
}

                               //character device file is written


static ssize_t dev_write(struct file *filep,char *buf,size_t len,loff_t *off){
        int length=0,i=0;
        resp_ready=1;
        if(wr_ready){
               wr_ready=0;
               copy_from_user(ker_buf,buf,len);
               //length=strlen(ker_buf);
               for(i=0;i<=8;i++){
                         length +=sscanf(ker_buf+length,"%d",x1+i);
                         printk(KERN_ALERT "length=%d",length);
                    
               }
               //sscanf(ker_buf,"%d,%d,%d,%d,%d,%d,%d,%d",&input_1,&input_2,&input_3,&input_4,&input_5,&input_6,&input_7,&input_8);

                                                //changr ip registers
             /*  *x1 = (unsigned int)input_1;
               *x2 = (unsigned int)input_2;
               *x3 = (unsigned int)input_3;
               *x4 = (unsigned int)input_4;
               *f1 = (unsigned int)input_5;
               *f2 = (unsigned int)input_6;
               *f3 = (unsigned int)input_7;
               *f4 = (unsigned int)input_8;*/
            
               //printk(KERN_ALERT "receiving values <%d,%d,%d,%d,%d,%d,%d,%d>\n",input_1,input_2,input_3,input_4,input_5,input_6,input_7,input_8);
               //printk(KERN_ALERT "receiving values <%d,%d,%d,%d,%d,%d,%d,%d>\n",*x1,*x2,*x3,*x4,*f1,*f2,*f3,*f4);
               printk(KERN_ALERT "****************Extra from atributes****************** \n");
            
               if(len>0){
                     printk(KERN_ALERT "Start IP Core\n");
                     ipStart();
                     while(!ipIsDone());
                     printk(KERN_ALERT "IP Core is done\n");
               }
            
               printk(KERN_ALERT "****************Extra from atributes****************** \n\n");
               return length;
        }
        else
               printk(KERN_ALERT "wr_ready is %d",wr_ready);
        return 0;               
}
     
     
                                 //character device file is read


static ssize_t dev_read(struct file *filep,char *buf,size_t len,loff_t *off) {
        int i;
        wr_ready=1;
        if(resp_ready==1){
               printk(KERN_ALERT "inputs are :x1:%d,x2:%d,x3:%d,x4:%d,\nf1:%d ,f2:%d,f3:%d,f4:%d \n",*x1,*x2,*x3,*x4,*f1,*f2,*f3,*f4);
               int length=0;
               for(i=0;i<=24;i++){
                         length +=sprintf(ker_buf+length,"0x%x:%d\n",output1+i,*(output1+i));
                         printk(KERN_ALERT "%d: length of buffer used : %d\n ,Result at (output1+i)0x%x location is %d  \n",i,length,output1+i,*(output1+i));
               }
               resp_ready=0;
               copy_to_user(buf,ker_buf,len);
               return length;
        }
        else
               printk(KERN_ALERT "resp_ready is %d \n", resp_ready);
        return 0;
}

static int dev_release(struct inode *inod ,struct file *fil){
         printk(KERN_ALERT "device closed\n");
         return 0;
}


module_init(interpolation_init);
module_exit(interpolation_exit);
