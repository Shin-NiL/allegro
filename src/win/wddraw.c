/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      DirectDraw gfx drivers.
 *
 *      By Stefan Schimanski.
 *
 *      See readme.txt for copyright information.
 */


#include "wddraw.h"


/* DirectDraw globals */
LPDIRECTDRAW2 directdraw = NULL;
LPDIRECTDRAWCLIPPER ddclipper = NULL;
LPDIRECTDRAWPALETTE ddpalette = NULL;
LPDDPIXELFORMAT ddpixel_format = NULL;
DDCAPS ddcaps;

DDRAW_SURFACE *primary_surface = NULL;
BITMAP *forefront_bitmap = NULL;
char *pseudo_surf_mem;

/* DirectDraw internals */
static PALETTEENTRY palette_entry[256];



/* init_directx:
 *  Low-level DirectDraw initialization routine.
 */
int init_directx(void)
{
   LPDIRECTDRAW directdraw1 = NULL;
   HRESULT hr;

   /* first we have to setup the DirectDraw2 interface */
   hr = DirectDrawCreate(NULL, &directdraw1, NULL);
   if (FAILED(hr))
      return -1;

   hr = IDirectDraw_SetCooperativeLevel(directdraw1, allegro_wnd, DDSCL_NORMAL);
   if (FAILED(hr))
      return -1;

   hr = IDirectDraw_QueryInterface(directdraw1, &IID_IDirectDraw2, (LPVOID *)&directdraw);
   if (FAILED(hr))
      return -1;

   IDirectDraw_Release(directdraw1);

   /* get capabilities */
   ddcaps.dwSize = sizeof(ddcaps);
   hr = IDirectDraw2_GetCaps(directdraw, &ddcaps, NULL);
   if (FAILED(hr)) {
      _TRACE("Can't get driver caps\n");
      return -1;
   }

   return 0;
}



/* create_primary:
 *  Low-level DirectDraw screen creation routine.
 */
int create_primary(void)
{
   /* create primary surface */
   primary_surface = gfx_directx_create_surface(0, 0, NULL, DDRAW_SURFACE_PRIMARY);
   if (!primary_surface) {
      _TRACE("Can't create primary surface.\n");
      return -1;
   }

   return 0;
}



/* create_clipper:
 *  Low-level DirectDraw clipper creation routine.
 */
int create_clipper(HWND hwnd)
{
   HRESULT hr;

   hr = IDirectDraw2_CreateClipper(directdraw, 0, &ddclipper, NULL);
   if (FAILED(hr)) {
      _TRACE("Can't create clipper (%x)\n", hr);
      return -1;
   }

   hr = IDirectDrawClipper_SetHWnd(ddclipper, 0, hwnd);
   if (FAILED(hr)) {
      _TRACE("Can't set clipper window (%x)\n", hr);
      return -1;
   }

   return 0;
}



/* create_palette:
 *  Low-level DirectDraw palette creation routine.
 */
int create_palette(DDRAW_SURFACE *surf)
{
   HRESULT hr;
   int n;

   /* prepare palette entries */
   for (n = 0; n < 256; n++) {
      palette_entry[n].peFlags = PC_NOCOLLAPSE | PC_RESERVED;
   }

   hr = IDirectDraw2_CreatePalette(directdraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256, palette_entry, &ddpalette, NULL);
   if (FAILED(hr)) {
      _TRACE("Can't create palette (%x)\n", hr);
      return -1;
   }

   hr = IDirectDrawSurface2_SetPalette(surf->id, ddpalette);
     if (FAILED(hr)) {
      _TRACE("Can't set palette (%x)\n", hr);
      return -1;
   }

   surf->flags |= DDRAW_SURFACE_INDEXED;

   return 0;
}



/* setup_driver:
 *  Helper function for initializing the gfx driver.
 */
int setup_driver(GFX_DRIVER *drv, int w, int h, int color_depth)
{
   DDSCAPS ddsCaps;

   /* setup the driver structure */
   drv->w = w;
   drv->h = h;
   drv->linear = 1;
   ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
   IDirectDraw2_GetAvailableVidMem(directdraw, &ddsCaps, &drv->vid_mem, NULL);
   drv->vid_mem += w * h * BYTES_PER_PIXEL(color_depth);

   /* create our pseudo surface memory */
   pseudo_surf_mem = malloc(2048 * BYTES_PER_PIXEL(color_depth));

   /* modify the vtable to work with video memory */
   memcpy(&_screen_vtable, _get_vtable(color_depth), sizeof(_screen_vtable));
   _screen_vtable.unwrite_bank = gfx_directx_unwrite_bank;
   _screen_vtable.acquire = gfx_directx_lock;
   _screen_vtable.release = gfx_directx_unlock;
   _screen_vtable.created_sub_bitmap = gfx_directx_created_sub_bitmap;

   return 0;
}



/* finalize_directx_init:
 *  Low-level DirectDraw init finalization routine.
 */
int finalize_directx_init(void)
{
   HRESULT hr;
   long int freq;

   /* set current refresh rate */
   hr = IDirectDraw2_GetMonitorFrequency(directdraw, &freq);

   if (FAILED(hr))
      _set_current_refresh_rate(0);
   else
      _set_current_refresh_rate(freq);

   return 0;
}



/* exit_directx:
 *  Low-level DirectDraw shut down routine.
 */
int exit_directx(void)
{
   if (directdraw) {
      /* set cooperative level back to normal */
      IDirectDraw2_SetCooperativeLevel(directdraw, allegro_wnd, DDSCL_NORMAL);

      /* release DirectDraw interface */
      IDirectDraw2_Release(directdraw);

      directdraw = NULL;
   }

   return 0;
}



/* gfx_directx_init:
 */
struct BITMAP *gfx_directx_init(GFX_DRIVER *drv, int w, int h, int v_w, int v_h, int color_depth)
{
   if ((v_w != w && v_w != 0) || (v_h != h && v_h != 0)) {
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported virtual resolution"));
      return NULL;
   }

   /* set up DirectDraw */
   if (init_directx() != 0)
      goto Error;

   /* switch to fullscreen */
   if (set_video_mode(w, h, v_w, v_h, color_depth) != 0) {
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Can not set video mode"));
      goto Error;
   }

   if (finalize_directx_init() != 0)
      goto Error;

   /* create screen */
   if (create_primary() != 0)
      goto Error;

   /* set color format */
   if (color_depth == 8) {
      if (create_palette(primary_surface) != 0)
	 goto Error;
   }
   else {
      if (gfx_directx_update_color_format(primary_surface, color_depth) != 0)
         goto Error;
   }

   /* set gfx driver interface */
   if (setup_driver(drv, w, h, color_depth) != 0)
      goto Error;

   /* create forefront bitmap */
   forefront_bitmap = make_bitmap_from_surface(primary_surface, w, h, BMP_ID_VIDEO);

   return forefront_bitmap;

 Error:
   gfx_directx_exit(NULL);

   return NULL;
}



/* gfx_directx_set_palette:
 */
void gfx_directx_set_palette(AL_CONST struct RGB *p, int from, int to, int vsync)
{
   int n;

   /* convert into Windows format */
   for (n = from; n <= to; n++) {
      palette_entry[n].peRed = (p[n].r << 2) | ((p[n].r & 0x30) >> 4);
      palette_entry[n].peGreen = (p[n].g << 2) | ((p[n].g & 0x30) >> 4);
      palette_entry[n].peBlue = (p[n].b << 2) | ((p[n].b & 0x30) >> 4);
   }

   /* wait for vertical retrace */
   if (vsync)
      gfx_directx_sync();

   /* set the convert palette */
   IDirectDrawPalette_SetEntries(ddpalette, 0, from, to - from + 1, &palette_entry[from]);
}



/* gfx_directx_sync:
 *  wait for vertical sync
 */
void gfx_directx_sync(void)
{
   IDirectDraw2_WaitForVerticalBlank(directdraw, DDWAITVB_BLOCKBEGIN, NULL);
}



/* gfx_directx_exit:
 */
void gfx_directx_exit(struct BITMAP *bmp)
{ 
   _enter_critical();

   _set_current_refresh_rate(0);

   if (bmp)
      clear_bitmap(bmp);

   /* disconnect from the system driver */
   win_gfx_driver = NULL;

   /* destroy primary surface */
   if (primary_surface) {
      gfx_directx_destroy_surface(primary_surface);
      primary_surface = NULL;
      forefront_bitmap = NULL;
   }

   /* normally this list must be empty */
   unregister_all_ddraw_surfaces();

   /* destroy clipper */
   if (ddclipper) {
      IDirectDrawClipper_Release(ddclipper);
      ddclipper = NULL;
   }

   /* destroy palette */
   if (ddpalette) {
      IDirectDrawPalette_Release(ddpalette);
      ddpalette = NULL;
   }

   /* free pseudo memory */
   if (pseudo_surf_mem) {
      free(pseudo_surf_mem);
      pseudo_surf_mem = NULL;
   }

   /* before restoring video mode, hide window */
   set_display_switch_mode(SWITCH_PAUSE);
   system_driver->restore_console_state();
   restore_window_style();

   /* let the window thread set the coop level back
    * to normal and destroy the directdraw object
    */
   wnd_call_proc(exit_directx);

   _exit_critical();
}

