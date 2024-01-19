// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>

#include <pci/pci.h>

int main(void)
{
	unsigned short ptm_found = 0;
	char namebuf[1024], *name;
	struct pci_access *pacc;
	struct pci_cap *iter;
	struct pci_dev *dev;
	unsigned int c;
	int ret = 0;

	if (geteuid() != 0) {
		fprintf(stderr, "Need to run program with elevated privileges!\n");
		return 2;
	}

	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

	for (dev = pacc->devices; dev; dev = dev->next) {
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES |
			      PCI_FILL_CLASS |
			      PCI_FILL_CAPS |
			      PCI_FILL_EXT_CAPS);

		if (dev->device_class != PCI_CLASS_BRIDGE_HOST &&
		    dev->device_class != PCI_CLASS_BRIDGE_PCI)
			continue;

		iter = dev->first_cap;
		while (iter) {
			if (iter->id == PCI_EXT_CAP_ID_PTM)
				goto print_dev;

			iter = iter->next;
		}

		continue;

print_dev:
		c = pci_read_byte(dev, PCI_INTERRUPT_PIN); /* Read config register directly */

		printf("%04x:%02x:%02x.%d vendor=%04x device=%04x class=%04x irq=%d (pin %d) base0=%lx",
		       dev->domain, dev->bus, dev->dev, dev->func,
		       dev->vendor_id, dev->device_id, dev->device_class,
		       dev->irq, c, (long)dev->base_addr[0]);

		/* Look up and print the full name of the device */
		name = pci_lookup_name(pacc, namebuf, sizeof(namebuf),
				       PCI_LOOKUP_DEVICE, dev->vendor_id,
				       dev->device_id);
		printf(" (%s)\n", name);

		ptm_found = 1;
	}

	if (ptm_found) {
		printf("The above bridge devices reported having the PTM PCI capability\n");
		printf("Host system appears to support Precision Time Measurement\n");
		goto out;
	}

	ret = 1;
	printf("Host system has no bridges that reported the PTM PCI capability\n");

out:
	pci_cleanup(pacc);
	return ret;
}
