#ifndef __MULTIBOOT__

#define __MULTIBOOT__

typedef struct
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	unsigned long syms[4];
	unsigned long mmap_length;
	unsigned long mmap_addr;
	unsigned long drives_length;
	unsigned long drives_addr;
	unsigned long config_table;
	unsigned long boot_loader_name;
	unsigned long apm_table;
	unsigned long vbe_control_info;
	unsigned long vbe_mode_info;
	unsigned short vbe_mode;
	unsigned short vbe_interface_seg;
	unsigned short vbe_interface_off;
	unsigned short vbe_interface_len;
}multiboot;

typedef struct
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long res;
}modules_info;


/*------------------------------------------*/
/* VBE controller information.  */
typedef struct
{
  unsigned char signature[4];
  unsigned short version;
  unsigned long oem_string;
  unsigned long capabilities;
  unsigned long video_mode;
  unsigned short total_memory;
  unsigned short oem_software_rev;
  unsigned long oem_vendor_name;
  unsigned long oem_product_name;
  unsigned long oem_product_rev;
  unsigned char reserved[222];
  unsigned char oem_data[256];
} vbe_controler;

/* VBE mode information.  */
typedef struct
{
  unsigned short mode_attributes;
  unsigned char win_a_attributes;
  unsigned char win_b_attributes;
  unsigned short win_granularity;
  unsigned short win_size;
  unsigned short win_a_segment;
  unsigned short win_b_segment;
  unsigned long win_func;
  unsigned short bytes_per_scanline;

  /* >=1.2 */
  unsigned short x_resolution;
  unsigned short y_resolution;
  unsigned char x_char_size;
  unsigned char y_char_size;
  unsigned char number_of_planes;
  unsigned char bits_per_pixel;
  unsigned char number_of_banks;
  unsigned char memory_model;
  unsigned char bank_size;
  unsigned char number_of_image_pages;
  unsigned char reserved0;

  /* direct color */
  unsigned char red_mask_size;
  unsigned char red_field_position;
  unsigned char green_mask_size;
  unsigned char green_field_position;
  unsigned char blue_mask_size;
  unsigned char blue_field_position;
  unsigned char reserved_mask_size;
  unsigned char reserved_field_position;
  unsigned char direct_color_mode_info;

  /* >=2.0 */
  unsigned long phys_base;
  unsigned long reserved1;
  unsigned short reversed2;

  /* >=3.0 */
  unsigned short linear_bytes_per_scanline;
  unsigned char banked_number_of_image_pages;
  unsigned char linear_number_of_image_pages;
  unsigned char linear_red_mask_size;
  unsigned char linear_red_field_position;
  unsigned char linear_green_mask_size;
  unsigned char linear_green_field_position;
  unsigned char linear_blue_mask_size;
  unsigned char linear_blue_field_position;
  unsigned char linear_reserved_mask_size;
  unsigned char linear_reserved_field_position;
  unsigned long max_pixel_clock;

  unsigned char reserved3[189];
} vbe_mode;

#define MAX_MODULES 5

#endif

