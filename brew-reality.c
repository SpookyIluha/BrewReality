#include <libdragon.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#include <rspq_profile.h>
#include <malloc.h>
#include <math.h>

#include "Quaternion.h"
#include "Quaternion.c"
#include "camera.h"
#include "aircraft.h"
#include "vector.h"
#include "../../src/utils.h"

// Set this to 1 to enable rdpq debug output.
// The demo will only run for a single frame and stop.
#define DEBUG_RDP 0

static uint32_t animation = 3283;
static uint32_t texture_index = 0;
static camera_t camera;
aircraft_t aircraft;
static surface_t zbuffer;
static xm64player_t sfx_music;
static wav64_t sfx_jet;
uint64_t totalTicks;
uint32_t lastTicks;
#define CHANNEL_SFX 0
#define CHANNEL_MUSIC 2
//#define M_PI 3.14159

//static uint64_t frames = 0;

color_t skyboxcolors[4] = {
    {0xF0,0xF0,0xFF,0x76},
    {0x20,0x20,0x20,0xFF},
    {0xDB,0xCB,0xDC,0xFF},
    {0xA8,0xB8,0xC8,0xFF}
};

static GLuint textures[20];

static bool fog_enabled = true;

GLfloat environment_color[] = { 0.9f, 0.93f, 0.95f, 1.f };

static const GLfloat light_pos[8][4] = {
    { 1, 1, 0, 0 },
    { -1, 0, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, -1, 0 },
    { 8, 3, 0, 1 },
    { -8, 3, 0, 1 },
    { 0, 3, 8, 1 },
    { 0, 3, -8, 1 },
};

static const GLfloat light_diffuse[8][4] = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
};

static const char *texture_path[20] = {
    "rom:/beach01.ihq.sprite",
    "rom:/grass01.ihq.sprite",
    "rom:/water01.ihq.sprite",
    "rom:/beach02.ihq.sprite",
    "rom:/beach04.ihq.sprite",
    "rom:/beach03.ihq.sprite",
    "rom:/trees01.ihq.sprite",
    "rom:/tree.rgba32.sprite",
    "rom:/clouds.i4.sprite",
    "rom:/stars.i4.sprite",
    "rom:/jetnose01.ihq.sprite",
    "rom:/jetwings01.ihq.sprite",
    "rom:/jettop01.ihq.sprite",
    "rom:/jetbottom01.ihq.sprite",
    "rom:/env_sky.rgba16.sprite",

    "rom:/ui.meter.i4.sprite",
    "rom:/ui.crosshair.i4.sprite",
    "rom:/ui.crosshair2.i4.sprite",
    "rom:/ui.crosshorz.i4.sprite",
    "rom:/ui.crossvert.i4.sprite",
};

static model64_t *coastmodel;
static rspq_block_t *coastblock;

static model64_t *skymodel;
static rspq_block_t *skyblock;

static model64_t *aircraftmodel;
static rspq_block_t *aircraftblock;

static sprite_t *sprites[20];

void setup()
{
    camera.distance = -5.0f;
    camera.yaw = 0.0f;
    camera.pitch = 10.0f;
    aircraft.pos[0] = 0;
    aircraft.pos[1] = 50;
    aircraft.pos[2] = 0;
    aircraft.Velocity[0] = 0;
    aircraft.Velocity[1] = 0;
    aircraft.Velocity[2] = -18;
    Quaternion_setIdentity(&aircraft.rotation);

    zbuffer = surface_alloc(FMT_RGBA16, display_get_width(), display_get_height());

    for (uint32_t i = 0; i < 20; i++)
    {
        sprites[i] = sprite_load(texture_path[i]);
    }

    coastmodel = model64_load("rom:/coast.model64");
    skymodel = model64_load("rom:/skybox.model64");
    aircraftmodel = model64_load("rom:/mig29.model64");

    float aspect_ratio = (float)display_get_width() / (float)display_get_height();
    float near_plane = 3.5f;
    float far_plane = 200.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, aspect_ratio, near_plane, far_plane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float light_radius = 10.0f;

    for (uint32_t i = 0; i < 1; i++)
    {
        glEnable(GL_LIGHT0 + i);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, light_diffuse[i]);
        glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 2.0f/light_radius);
        glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 1.0f/(light_radius*light_radius));
    }

    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

    glFogf(GL_FOG_START, 70);
    glFogf(GL_FOG_END, 170);
    glFogfv(GL_FOG_COLOR, environment_color);

    glEnable(GL_MULTISAMPLE_ARB);
    //glHint(GL_MULTISAMPLE_HINT_N64, GL_FASTEST);
    glEnable(GL_DITHER);

    glGenTextures(20, textures);


    for (uint32_t i = 0; i < 15; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSpriteTextureN64(GL_TEXTURE_2D, sprites[i], NULL);
    }
}

void set_light_positions(float rotation)
{
    glPushMatrix();
    glRotatef(rotation*5.43f, 0, 1, 0);

    for (uint32_t i = 0; i < 8; i++)
    {
        glLightfv(GL_LIGHT0 + i, GL_POSITION, light_pos[i]);
    }
    glPopMatrix();
}

void audio_poll(){
        // Check whether one audio buffer is ready, otherwise wait for next
        // frame to perform mixing.
        if (audio_can_write()) {
            short *buf = audio_write_begin();
            mixer_poll(buf, audio_get_buffer_length());
            audio_write_end();
        }
}

int frame = 1;

void render()
{
    surface_t *disp = display_get();

    rdpq_attach(disp, &zbuffer);

    gl_context_begin();
    frame++;

    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glScalef(2,2,2);

    glMatrixMode(GL_MODELVIEW);
    camera_transform(&camera, &aircraft);

    float rotation = animation * 0.5f;

    set_light_positions(rotation);
    glBindTexture(GL_TEXTURE_2D, textures[9]);

    glPushMatrix();
    glTranslatef(aircraft.pos[0],aircraft.pos[1],aircraft.pos[2]);
    glScalef(10,10,10);
            environment_color[0] = ((float)skyboxcolors[3].r / 255);
        environment_color[1] = ((float)skyboxcolors[3].g / 255);
        environment_color[2] = ((float)skyboxcolors[3].b / 255);
        environment_color[3] = ((float)skyboxcolors[3].a / 255);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_FOG);
        glDisable(GL_MULTISAMPLE_ARB);
            glEnable(GL_RDPQ_MATERIAL_N64);
            glEnable(GL_RDPQ_TEXTURING_N64);

            rdpq_tex_multi_begin();
            surface_t surf = sprite_get_pixels(sprites[8]);
            rdpq_tex_load(TILE0, &surf, &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE,
            .s.translate = (animation % 8192) * 0.008f, .t.translate = (animation % 8192) * 0.04f, .s.scale_log = -1, .t.scale_log = -1});
            surf = sprite_get_pixels(sprites[9]);
            rdpq_tex_load(TILE1, &surf, &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE, .s.scale_log = -1, .t.scale_log = -1, .t.mirror = true});
            rdpq_tex_multi_end();
            rdpq_mode_filter(FILTER_BILINEAR);
            rdpq_mode_mipmap(MIPMAP_NONE,0);

            rdpq_mode_combiner(RDPQ_COMBINER2((SHADE,0,ENV,0),(TEX1,0,SHADE,0),
                                            (PRIM,0,COMBINED_ALPHA,COMBINED),(TEX0,ENV,SHADE,0)));

            rdpq_mode_blender(RDPQ_BLENDER2((BLEND_RGB,IN_ALPHA,IN_RGB,INV_MUX_ALPHA),
                                            (CYCLE1_RGB,SHADE_ALPHA,FOG_RGB,INV_MUX_ALPHA)));

            rdpq_set_env_color(skyboxcolors[0]);
            rdpq_set_prim_color(skyboxcolors[1]);
            rdpq_set_blend_color(skyboxcolors[2]);
            rdpq_set_fog_color(skyboxcolors[3]);
    if(!skyblock){
            rspq_block_begin();
            model64_draw(skymodel);

            glEnable(GL_MULTISAMPLE_ARB);
            glDisable(GL_RDPQ_MATERIAL_N64);
            glDisable(GL_RDPQ_TEXTURING_N64);
        skyblock = rspq_block_end();
    } rspq_block_run(skyblock);

    audio_poll();

        glPopMatrix();
    // Set some global render modes that we want to apply to all models
    glEnable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[texture_index]);

    if(!coastblock){
        rspq_block_begin();
        mesh_t *mesh = model64_get_mesh(coastmodel, 0);
        for(int i = 0; i < model64_get_primitive_count(mesh); i++){
            primitive_t *prim = model64_get_primitive(mesh, i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            if(i == 7) {
                glDisable(GL_MULTISAMPLE_ARB);
                glEnable(GL_BLEND);
                glDisable(GL_CULL_FACE);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_DEPTH_TEST); 
                glDepthMask(GL_FALSE);
            }
            model64_draw_primitive(prim);
            glEnable(GL_MULTISAMPLE_ARB);
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST); 
            glDepthMask(GL_TRUE);
        }
        coastblock = rspq_block_end();
    } rspq_block_run(coastblock);

    audio_poll();

    glPushMatrix();
    glTranslatef(aircraft.pos[0],aircraft.pos[1],aircraft.pos[2]);
    glRotatef(aircraft.rot[2] * 180.0f / M_PI, 0,0,1);
    glRotatef(aircraft.rot[1] * 180.0f / M_PI, 0,1,0);
    glRotatef(aircraft.rot[0] * 180.0f / M_PI, 1,0,0);

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if(!aircraftblock){
        rspq_block_begin();
        mesh_t *mesh = model64_get_mesh(aircraftmodel, 0);
        for(int i = 0; i < model64_get_primitive_count(mesh); i++){
            primitive_t *prim = model64_get_primitive(mesh, i);
            glBindTexture(GL_TEXTURE_2D, textures[i+10]);
            model64_draw_primitive(prim);
        }
        aircraftblock = rspq_block_end();
    } rspq_block_run(aircraftblock);
    glPopMatrix();


    gl_context_end();

    rdpq_text_printf(&(rdpq_textparms_t){
            .align = ALIGN_LEFT,
            .valign = VALIGN_TOP,
            .width = 100,
            .height = 50,
            .wrap = WRAP_WORD,
    }, 1, 60, 40, "%.2f FPS", display_get_fps());

    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_antialias(AA_STANDARD);
    rdpq_mode_alphacompare(1);

    rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,0),(0,0,0,TEX0)));
    rdpq_mode_blender(RDPQ_BLENDER((BLEND_RGB,IN_ALPHA,MEMORY_RGB,INV_MUX_ALPHA)));
    rdpq_set_blend_color(RGBA32(0x63,0xdf,0,0x01));

    rdpq_sprite_blit(sprites[16], 220, 200, NULL);
    rdpq_sprite_blit(sprites[17], 320, 240, &(rdpq_blitparms_t){.cx = 100, .cy = 10, .theta = aircraft.roll}); 

    rdpq_sprite_upload(TILE0, sprites[15], &(rdpq_texparms_t){.t.translate = (float)((int)((aircraft.pos[1])*4*4)&0x1ff) / 4, .s.mirror = true, .s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});
    rdpq_texture_rectangle(TILE0, 160, 120, 160 + 32, 350, 0,0); 
    rdpq_texture_rectangle(TILE0, 440, 120, 440 + 32, 350, 16,0);  
    rdpq_sprite_blit(sprites[18], 140, 232, NULL);

    rdpq_sprite_upload(TILE0, sprites[15], &(rdpq_texparms_t){.t.translate = (float)((int)((aircraft.yaw)*(180/M_PI)*4)&0x1ff) / 4, .s.mirror = true, .s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});
    rdpq_texture_rectangle_flip_raw(TILE0, 200, 100, 440,100 + 32, 32,0, 1,1);   
    rdpq_sprite_blit(sprites[19], 312, 90, NULL);

    rdpq_text_printf(&(rdpq_textparms_t){
            .align = ALIGN_RIGHT,
            .valign = VALIGN_CENTER,
            .width = 100,
            .height = 50,
            .wrap = WRAP_WORD,
    }, 1, 30, 215, "%.2fm", aircraft.pos[1]);

    audio_poll();
    
    rdpq_detach_show();
}

int main()
{
#if DEBUG_RDP
	debug_init_isviewer();
	debug_init_usblog();
#endif
    
    dfs_init(DFS_DEFAULT_LOCATION);

    display_init((resolution_t){640, 480, INTERLACE_HALF}, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER); 

    rdpq_init();
    gl_init();
    timer_init();

    audio_init(32000, 4);
	mixer_init(12);

    xm64player_open(&sfx_music, "rom:/kc-basslinetech.xm64"); 
    xm64player_set_loop(&sfx_music, true);
    xm64player_set_vol(&sfx_music, 0.35);
    xm64player_play(&sfx_music, CHANNEL_MUSIC);

    wav64_open(&sfx_jet, "rom:/jetsound.wav64"); 
    wav64_set_loop(&sfx_jet, true);
    mixer_ch_set_vol(CHANNEL_SFX,  0.1, 0.1);
    wav64_play(&sfx_jet, CHANNEL_SFX);

    rdpq_font_t *fnt1 = rdpq_font_load("rom:/FerriteCoreDX.font64");
    rdpq_font_style(fnt1, 0, &(rdpq_fontstyle_t){
        .color = RGBA32(0x63,0xdf,0,0xff), 
    });
    rdpq_text_register_font(1, fnt1);

#if DEBUG_RDP
    rdpq_debug_start();
    rdpq_debug_log(true);
#endif

    lastTicks = get_ticks();

    setup();
    joypad_init();

#if !DEBUG_RDP
    while (1)
#endif
    {
        if(1){
            double factor = 1.0f / ((double)TICKS_PER_SECOND);

            uint32_t currentTicks = get_ticks();
            totalTicks += TICKS_DISTANCE(lastTicks, currentTicks);
            double deltatime = (currentTicks - lastTicks) * factor;
            lastTicks = currentTicks;

            joypad_poll();
            joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
            joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_1);

            if(inputs.btn.c_up){
                camera.pitch += 30.0f*deltatime;
            }
            if(inputs.btn.c_down){
                camera.pitch -= 30.0f*deltatime;
            }
            if(inputs.btn.c_left){
                camera.yaw -= 30.0f*deltatime;
            }
            if(inputs.btn.c_right){
                camera.yaw += 30.0f*deltatime;
            }

            if (pressed.l) {
                fog_enabled = !fog_enabled;
                if (fog_enabled) {
                    glEnable(GL_FOG);
                } else {
                    glDisable(GL_FOG);
                }
            }

            float y = inputs.stick_y / 128.f;
            float x = inputs.stick_x / 128.f;
            float mag = x*x + y*y;

            if (fabsf(mag) > 0.01f) {
                Quaternion RollQuat;
                Quaternion PitchQuat;

                Quaternion_fromZRotation(-1.8f*x*deltatime, &RollQuat);
                Quaternion_fromXRotation(-0.9f*y*deltatime, &PitchQuat);

                Quaternion_multiply(&aircraft.rotation,&RollQuat, &aircraft.rotation);
                Quaternion_multiply(&aircraft.rotation,&PitchQuat, &aircraft.rotation);
                Quaternion_normalize(&aircraft.rotation, &aircraft.rotation);
            }
                Quaternion_rotate(&aircraft.rotation, aircraft.Velocity, aircraft.fDirection);
                vector_scale(aircraft.fDirection, deltatime);
                vector_add(aircraft.pos, aircraft.fDirection, aircraft.pos);
                Quaternion_toEulerZYX(&aircraft.rotation, aircraft.rot);
                aircraft.roll = aircraft.rot[2];
                aircraft.yaw = aircraft.rot[0];
            animation--;
            animation = animation % 8192;
        }

        render();
    }

}
