#ifndef __ORIONFB_HW_H__
#define __ORIONFB_HW_H__

void df_writel(int addr, int val);
int df_readl(int addr);

int orionfb_hw_init(struct fb_info *info, int en_flags);
void orionfb_hw_exit(void);

int orionfb_hw_set_par(struct fb_info *info);
int orionfb_hw_pan_display(struct fb_info *info);
int orionfb_hw_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
                         u_int transp, struct fb_info *info);

int orionfb_hw_blank(int blank);
int orionfb_hw_waitforvsync(struct fb_info *info);

int orionfb_hw_gfx_on(struct fb_info *info, unsigned int args);
int orionfb_hw_gfx_alpha(struct fb_info *info, unsigned int args);

int orionfb_hw_z_order(struct fb_info *info,unsigned int arg);

int orionfb_hw_colorkey_val(struct fb_info *info, gfx_colorkey *col_key);
int orionfb_hw_colorkey_on(struct fb_info *info, unsigned int arg);

#endif /* __ORIONFB_HW_H__ */
