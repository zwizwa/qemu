#include "hw/hw.h"
#include "hw/sysbus.h"
#include "hw/boards.h"
#include "hw/devices.h"
#include "strongarm.h"
#include "hw/arm/arm.h"
#include "hw/block/flash.h"
#include "sysemu/blockdev.h"
#include "exec/address-spaces.h"

typedef struct {
    SysBusDevice busdev;
    MemoryRegion iomem;
} zwizwa_debug_state;
static uint64_t zwizwa_debug_read(void *opaque, hwaddr offset, unsigned size) {
    //printf("zwizwa read: %08x\n", (unsigned int)offset);
    return 0;
}
static void zwizwa_debug_write(void *opaque, hwaddr offset, uint64_t value, unsigned size) {
    // printf("zwizwa_debug_write()\n");
    // printf("zwizwa write: %08x <- %08x\n", (unsigned int)offset, (unsigned int)value);
    fputc(value, stderr);
}
static const MemoryRegionOps zwizwa_debug_ops = {
    .read  = zwizwa_debug_read,
    .write = zwizwa_debug_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};
static int zwizwa_debug_init(SysBusDevice *dev){
    printf("zwizwa_debug_init()\n");
    zwizwa_debug_state *s = DO_UPCAST(typeof(*s), busdev, dev);
    memory_region_init_io(&s->iomem, NULL, &zwizwa_debug_ops, s, "zwizwa-debug-io", 32);
    sysbus_init_mmio(dev, &s->iomem);
    return 0;
}
static void zwizwa_debug_reset(DeviceState *d){
    zwizwa_debug_state *s = container_of(d, typeof(*s), busdev.parent_obj);
}
static void zwizwa_debug_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    SysBusDeviceClass *sdc = SYS_BUS_DEVICE_CLASS(klass);
    sdc->init = zwizwa_debug_init;
    dc->reset = zwizwa_debug_reset;
}
static TypeInfo zwizwa_debug_info = {
    .name          = "zwizwa-debug",
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(zwizwa_debug_state),
    .class_init    = zwizwa_debug_class_init,
};
static void zwizwa_register_types(void) {
    type_register_static(&zwizwa_debug_info);
}
type_init(zwizwa_register_types)




struct arm_boot_info zwizwa_binfo;

static void zwizwa_init(QEMUMachineInitArgs *args)
{
    MemoryRegion *sysmem = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);

    if (!args->cpu_model) {
        args->cpu_model = "arm926";
    }

    ARMCPU *c = cpu_arm_init(args->cpu_model);
    if (!c) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    memory_region_init_ram(ram, NULL, "zwizwa.ram", 16*1024);
    vmstate_register_ram_global(ram);
    memory_region_add_subregion(sysmem, 0x0, ram);

    zwizwa_binfo.ram_size = args->ram_size;
    zwizwa_binfo.kernel_filename = args->kernel_filename;

    sysbus_create_varargs("zwizwa-debug", 0x0100000, NULL);

    arm_load_kernel(c, &zwizwa_binfo);

}

static QEMUMachine zwizwa_machine = {
    .name = "zwizwa",
    .desc = "Zwizwa test machine",
    .init = zwizwa_init,
};

static void zwizwa_machine_init(void)
{
    qemu_register_machine(&zwizwa_machine);
}

machine_init(zwizwa_machine_init)
