ccflags-$(CONFIG_TEE_DRIVER) += -I$(srctree)/drivers/hisilicon/sec_mmz
obj-$(CONFIG_TEE_DRIVER) += ion_tee_heap.o
obj-$(CONFIG_ION) +=	ion.o ion_heap.o ion_page_pool.o ion_system_heap.o \
			ion_carveout_heap.o ion_chunk_heap.o ion_cma_heap.o
ifdef CONFIG_COMPAT
obj-$(CONFIG_ION) += compat_ion.o
endif
obj-$(CONFIG_ION_TEGRA) += tegra/
obj-$(CONFIG_ION_HISI) += hisi/
