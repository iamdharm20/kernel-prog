#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>

void try_iowrite32(void) {
  void __iomem *p = (void *) 0x12345678;

  iowrite32(0xabcd0001, p);
  iowrite32(0xabcd0001, p);
  iowrite32(0xabcd0002, p);
  mmiowb();
  iowrite32(0xabcd0003, p);
  wmb();
  iowrite32(0xabcd0004, p);
  rmb();
  iowrite32(0xabcd0005, p);
  smp_wmb();
  iowrite32(0xabcd0006, p);
  smp_rmb();
}

EXPORT_SYMBOL(try_iowrite32);
