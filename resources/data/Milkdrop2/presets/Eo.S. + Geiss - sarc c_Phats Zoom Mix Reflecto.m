MILKDROP_PRESET_VERSION=201
PSVERSION=2
PSVERSION_WARP=0
PSVERSION_COMP=2
[preset00]
fRating=3.000000
fGammaAdj=2.000
fDecay=0.960
fVideoEchoZoom=1.007
fVideoEchoAlpha=0.000
nVideoEchoOrientation=1
nWaveMode=2
bAdditiveWaves=0
bWaveDots=1
bWaveThick=0
bModWaveAlphaByVolume=0
bMaximizeWaveColor=0
bTexWrap=0
bDarkenCenter=0
bRedBlueStereo=0
bBrighten=0
bDarken=1
bSolarize=0
bInvert=0
fWaveAlpha=0.001
fWaveScale=0.012
fWaveSmoothing=0.900
fWaveParam=0.000
fModWaveAlphaStart=0.750
fModWaveAlphaEnd=0.950
fWarpAnimSpeed=0.010
fWarpScale=0.010
fZoomExponent=1.00000
fShader=0.000
zoom=1.00000
rot=0.00000
cx=0.500
cy=0.500
dx=0.00000
dy=0.00000
warp=0.01000
sx=1.00000
sy=1.00000
wave_r=0.500
wave_g=0.400
wave_b=0.300
wave_x=0.500
wave_y=0.500
ob_size=0.000
ob_r=0.110
ob_g=0.000
ob_b=0.100
ob_a=0.000
ib_size=0.000
ib_r=0.000
ib_g=0.000
ib_b=0.000
ib_a=1.000
nMotionVectorsX=64.000
nMotionVectorsY=48.000
mv_dx=0.000
mv_dy=0.000
mv_l=5.000
mv_r=0.000
mv_g=0.000
mv_b=0.000
mv_a=0.000
b1n=0.000
b2n=0.000
b3n=0.000
b1x=1.000
b2x=1.000
b3x=1.000
b1ed=0.250
wavecode_0_enabled=1
wavecode_0_samples=512
wavecode_0_sep=0
wavecode_0_bSpectrum=0
wavecode_0_bUseDots=0
wavecode_0_bDrawThick=0
wavecode_0_bAdditive=1
wavecode_0_scaling=1.00000
wavecode_0_smoothing=0.50000
wavecode_0_r=0.100
wavecode_0_g=0.280
wavecode_0_b=1.000
wavecode_0_a=0.200
wave_0_init1=t2 = 0
wave_0_per_frame1=t1 = time * 13;
wave_0_per_frame2=
wave_0_per_frame3=t2 = sin(time*3) * 0.5 + 0.5;
wave_0_per_frame4=t2 = t2 * 0.3 + 0.1; //size of circle;
wave_0_per_frame5=
wave_0_per_frame6=t2= q2 * 0.003 + 0.06;
wave_0_per_frame7=
wave_0_per_frame8=t3= q1;
wave_0_per_frame9=
wave_0_per_frame10=t4 = time/8;
wave_0_per_frame11=
wave_0_per_frame12=t5 = sin( time / 4 ) * 0.5 + 0.5; //bias value
wave_0_per_frame13=t5 = t5 * 17 + 2;
wave_0_per_point1=n = sample*6.283;
wave_0_per_point2=pi = 3.1415;
wave_0_per_point3=pi2 = 6.283;
wave_0_per_point4=
wave_0_per_point5=phs = t1 + sample*9;
wave_0_per_point6=
wave_0_per_point7=bias = t5;
wave_0_per_point8=bias_i = bias - 1;
wave_0_per_point9=
wave_0_per_point10=cc = phs / 3;
wave_0_per_point11=cc_int = int(cc);
wave_0_per_point12=cc_ramp = cc - cc_int;
wave_0_per_point13=cc_ad_a = (cc_ramp * bias - 1) / bias_i;
wave_0_per_point14=cc_ad_a = if( below(cc_ad_a,0) , 0 , cc_ad_a );
wave_0_per_point15=
wave_0_per_point16=cc_ad_b = cc_ramp * bias;
wave_0_per_point17=cc_ad_b = if( above(cc_ad_b,1) , 1 , cc_ad_b );
wave_0_per_point18=
wave_0_per_point19=cc_a = cc_ad_a + cc_int;
wave_0_per_point20=cc_b = (cc_ad_b + cc_int) ;
wave_0_per_point21=
wave_0_per_point22=xp = t2 * above(cc_ad_a , 0);
wave_0_per_point23=yp = 1;
wave_0_per_point24=zp = 0;
wave_0_per_point25=
wave_0_per_point26=ang = cc_a * 6.283 * 16;
wave_0_per_point27=sang = sin(ang) ; cang = cos(ang);
wave_0_per_point28=xq = sang*xp + cang*zp;
wave_0_per_point29=yq = yp;
wave_0_per_point30=zq = cang*xp - sang*zp;
wave_0_per_point31=xp=xq;yp=yq;zp=zq;
wave_0_per_point32=
wave_0_per_point33=ang2 = cc_b + t4;
wave_0_per_point34=sang = sin(ang2) ; cang = cos(ang2);
wave_0_per_point35=xq = xp;
wave_0_per_point36=yq = sang*yp + cang*zp;
wave_0_per_point37=zq = cang*yp - sang*zp;
wave_0_per_point38=xp=xq;yp=yq;zp=zq;
wave_0_per_point39=
wave_0_per_point40=ang2 = cc_b * 3.13 + t4;
wave_0_per_point41=sang = sin(ang2) ; cang = cos(ang2);
wave_0_per_point42=xq = sang*xp + cang*yp;
wave_0_per_point43=yq = cang*xp - sang*yp;
wave_0_per_point44=zq = zp;
wave_0_per_point45=xp=xq;yp=yq;zp=zq;
wave_0_per_point46=
wave_0_per_point47=ang2 = cc_b * 1.43 + t4;
wave_0_per_point48=sang = sin(ang2) ; cang = cos(ang2);
wave_0_per_point49=xq = sang*xp + cang*zp;
wave_0_per_point50=yq = yp;
wave_0_per_point51=zq = cang*xp - sang*zp;
wave_0_per_point52=xp=xq;yp=yq;zp=zq;
wave_0_per_point53=
wave_0_per_point54=
wave_0_per_point55=zp = zp+3.1;
wave_0_per_point56=xs = xp/zp + 0.5;
wave_0_per_point57=ys = yp/zp * 1.333 + 0.5;
wave_0_per_point58=
wave_0_per_point59=x=xs;
wave_0_per_point60=y=ys;
wave_0_per_point61=
wave_0_per_point62=
wavecode_1_enabled=0
wavecode_1_samples=512
wavecode_1_sep=0
wavecode_1_bSpectrum=0
wavecode_1_bUseDots=0
wavecode_1_bDrawThick=0
wavecode_1_bAdditive=0
wavecode_1_scaling=1.00000
wavecode_1_smoothing=0.50000
wavecode_1_r=1.000
wavecode_1_g=1.000
wavecode_1_b=1.000
wavecode_1_a=1.000
wavecode_2_enabled=0
wavecode_2_samples=512
wavecode_2_sep=0
wavecode_2_bSpectrum=0
wavecode_2_bUseDots=0
wavecode_2_bDrawThick=0
wavecode_2_bAdditive=0
wavecode_2_scaling=1.00000
wavecode_2_smoothing=0.50000
wavecode_2_r=1.000
wavecode_2_g=1.000
wavecode_2_b=1.000
wavecode_2_a=1.000
wavecode_3_enabled=0
wavecode_3_samples=512
wavecode_3_sep=0
wavecode_3_bSpectrum=0
wavecode_3_bUseDots=0
wavecode_3_bDrawThick=0
wavecode_3_bAdditive=0
wavecode_3_scaling=1.00000
wavecode_3_smoothing=0.50000
wavecode_3_r=1.000
wavecode_3_g=1.000
wavecode_3_b=1.000
wavecode_3_a=1.000
shapecode_0_enabled=1
shapecode_0_sides=22
shapecode_0_additive=1
shapecode_0_thickOutline=0
shapecode_0_textured=1
shapecode_0_num_inst=1
shapecode_0_x=0.500
shapecode_0_y=0.500
shapecode_0_rad=0.54822
shapecode_0_ang=1.44513
shapecode_0_tex_ang=0.00000
shapecode_0_tex_zoom=1.00000
shapecode_0_r=0.300
shapecode_0_g=0.910
shapecode_0_b=1.000
shapecode_0_a=0.220
shapecode_0_r2=0.000
shapecode_0_g2=0.000
shapecode_0_b2=0.000
shapecode_0_a2=0.000
shapecode_0_border_r=1.000
shapecode_0_border_g=1.000
shapecode_0_border_b=1.000
shapecode_0_border_a=0.000
shapecode_1_enabled=1
shapecode_1_sides=33
shapecode_1_additive=0
shapecode_1_thickOutline=0
shapecode_1_textured=0
shapecode_1_num_inst=1
shapecode_1_x=0.500
shapecode_1_y=0.500
shapecode_1_rad=0.24979
shapecode_1_ang=0.00000
shapecode_1_tex_ang=0.00000
shapecode_1_tex_zoom=1.00000
shapecode_1_r=0.400
shapecode_1_g=0.200
shapecode_1_b=0.000
shapecode_1_a=1.000
shapecode_1_r2=0.100
shapecode_1_g2=0.200
shapecode_1_b2=0.300
shapecode_1_a2=1.000
shapecode_1_border_r=1.000
shapecode_1_border_g=1.000
shapecode_1_border_b=1.000
shapecode_1_border_a=0.000
shapecode_2_enabled=1
shapecode_2_sides=3
shapecode_2_additive=1
shapecode_2_thickOutline=0
shapecode_2_textured=1
shapecode_2_num_inst=1
shapecode_2_x=0.500
shapecode_2_y=0.500
shapecode_2_rad=0.36457
shapecode_2_ang=1.57080
shapecode_2_tex_ang=0.00000
shapecode_2_tex_zoom=1.41660
shapecode_2_r=1.000
shapecode_2_g=1.000
shapecode_2_b=1.000
shapecode_2_a=1.000
shapecode_2_r2=1.000
shapecode_2_g2=1.000
shapecode_2_b2=1.000
shapecode_2_a2=0.000
shapecode_2_border_r=1.000
shapecode_2_border_g=1.000
shapecode_2_border_b=1.000
shapecode_2_border_a=0.180
shape_2_per_frame1=ang = time/4
shapecode_3_enabled=1
shapecode_3_sides=3
shapecode_3_additive=0
shapecode_3_thickOutline=0
shapecode_3_textured=1
shapecode_3_num_inst=1
shapecode_3_x=0.500
shapecode_3_y=0.500
shapecode_3_rad=0.10000
shapecode_3_ang=0.00000
shapecode_3_tex_ang=0.00000
shapecode_3_tex_zoom=1.00000
shapecode_3_r=1.000
shapecode_3_g=1.000
shapecode_3_b=1.000
shapecode_3_a=1.000
shapecode_3_r2=0.500
shapecode_3_g2=0.700
shapecode_3_b2=1.000
shapecode_3_a2=0.000
shapecode_3_border_r=1.000
shapecode_3_border_g=1.000
shapecode_3_border_b=1.000
shapecode_3_border_a=0.100
shape_3_per_frame1=ang = - time/3
per_frame_init_1=mv_x=64;mv_y=48;
per_frame_init_2=nut=0;
per_frame_init_3=stp=0;stq=0;
per_frame_init_4=rtp=0;rtq=0;
per_frame_init_5=wvr=0;
per_frame_init_6=decay=0;
per_frame_init_7=dcsp=0
per_frame_init_8=
per_frame_1=decay=0.95;
per_frame_2=zoom=1.002;
per_frame_3=
per_frame_4=
per_frame_5=q1= ib_a * 6.283;
per_frame_6=
per_frame_7=q2 = (bass+mid+treb)*0.25;
per_frame_8=q2 = q2*q2;
per_frame_9=
per_frame_10=monitor = q2;
per_frame_11=
per_frame_12=
per_frame_13=
per_frame_14=
per_pixel_1=zoom=1+(rad/7);
per_pixel_2=rot=(rad/100)*sin(time);
comp_1=`shader_body
comp_2=`{
comp_3=`    //uv = lerp(uv, float2(rad,uv.x), roam_cos.x);
comp_4=`    //uv = lerp(uv, float2(uv.y,rad), roam_cos.y);
comp_5=`    uv = float2(rad,uv.y);
comp_6=`
comp_7=`    ret = tex2D(sampler_main, uv).xyz;
comp_8=`    ret += GetBlur1(uv);
comp_9=`    ret *= float3(1.3,0.8,0.5)*0.8;    
comp_10=`}
