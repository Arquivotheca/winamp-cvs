MILKDROP_PRESET_VERSION=201
PSVERSION=2
PSVERSION_WARP=2
PSVERSION_COMP=0
[preset00]
fRating=4.000000
fGammaAdj=1.0
fDecay=0.9
fVideoEchoZoom=0.999607
fVideoEchoAlpha=0.0
nVideoEchoOrientation=1
nWaveMode=7
bAdditiveWaves=1
bWaveDots=0
bWaveThick=0
bModWaveAlphaByVolume=1
bMaximizeWaveColor=0
bTexWrap=0
bDarkenCenter=0
bRedBlueStereo=0
bBrighten=1
bDarken=0
bSolarize=0
bInvert=0
fWaveAlpha=4.099998
fWaveScale=1.285751
fWaveSmoothing=0.630000
fWaveParam=0.0
fModWaveAlphaStart=0.710000
fModWaveAlphaEnd=1.3
fWarpAnimSpeed=1.0
fWarpScale=1.331000
fZoomExponent=0.999836
fShader=0.0
zoom=0.999511
rot=0.0
cx=0.5
cy=0.5
dx=0.0
dy=0.0
warp=0.010000
sx=1.0
sy=1.0
wave_r=0.650000
wave_g=0.650000
wave_b=0.650000
wave_x=0.5
wave_y=0.5
ob_size=0.005000
ob_r=0.0
ob_g=0.0
ob_b=0.0
ob_a=1.0
ib_size=0.260000
ib_r=0.25
ib_g=0.25
ib_b=0.25
ib_a=0.0
nMotionVectorsX=12.0
nMotionVectorsY=9.0
mv_dx=0.0
mv_dy=0.0
mv_l=0.9
mv_r=1.0
mv_g=1.0
mv_b=1.0
mv_a=0.0
b1n=0.0
b2n=0.0
b3n=0.0
b1x=1.0
b2x=1.0
b3x=1.0
b1ed=0.25
wavecode_0_enabled=1
wavecode_0_samples=512
wavecode_0_sep=0
wavecode_0_bSpectrum=1
wavecode_0_bUseDots=0
wavecode_0_bDrawThick=1
wavecode_0_bAdditive=0
wavecode_0_scaling=1.0
wavecode_0_smoothing=0.5
wavecode_0_r=1.0
wavecode_0_g=1.0
wavecode_0_b=1.0
wavecode_0_a=1.0
wave_0_per_frame1=ps = if(above(time-tin,tr),1,0);
wave_0_per_frame2=tin = if(ps,time,tin);
wave_0_per_frame3=
wave_0_per_frame4=sz = 3.5;
wave_0_per_frame5=
wave_0_per_frame6=xr = if(ps,rand(sz*10*2 + 1)*.1 - sz,xr);
wave_0_per_frame7=yr = if(ps,rand(sz*10*2*.75 + 1)*.1 - sz*.75,yr);
wave_0_per_frame8=tr = if(ps,rand(11)*.1*.1,tr);
wave_0_per_frame9=
wave_0_per_frame10=sr = if(ps,(rand(8)+3)*.1*.6,sr);
wave_0_per_frame11=
wave_0_per_frame12=bob = if(ps,rand(q5)*100,bob);
wave_0_per_frame13=
wave_0_per_frame14=t1 = xr + bob;
wave_0_per_frame15=t2 = yr + bob;
wave_0_per_frame16=t3 = sr;
wave_0_per_point1=sp = sample*6.28*8*8*4;
wave_0_per_point2=it = it+1;
wave_0_per_point3=it = it*above(sample,0);
wave_0_per_point4=sz = t3;
wave_0_per_point5=ss = sample*6;
wave_0_per_point6=ox = sz*.5*pow(-1,it)*below(ss,1) + .5*pow(-1,it)*above(ss,1)*sz*below(ss,2) + above(ss,2)*.5*pow(-1,it)*sz*below(ss,3);
wave_0_per_point7=oy = (ss-.5)*sz*below(ss,1) + sz*.5*above(ss,1)*below(ss,2) + (.5-(ss-2))*sz*above(ss,2)*below(ss,3);
wave_0_per_point8=oz = -sz*.5*below(ss,1) + ((ss-1)-.5)*sz*above(ss,1)*below(ss,2) + sz*.5*above(ss,2)*below(ss,3);
wave_0_per_point9=ox = ox + above(ss,3)*below(ss,4)*-.5*sz + above(ss,4)*below(ss,5)*sz*(-.5+(ss-4)) + above(ss,5)*sz*.5;
wave_0_per_point10=oy = oy + above(ss,3)*below(ss,4)*.5*sz*pow(-1,it) + above(ss,4)*below(ss,5)*sz*-.5 + above(ss,5)*sz*(-.5+(ss-5));
wave_0_per_point11=oz = oz + above(ss,3)*below(ss,4)*sz*(.5-(ss-3)) + above(ss,4)*below(ss,5)*sz*.5*pow(-1,it) + above(ss,5)*sz*.5*pow(-1,it);
wave_0_per_point12=
wave_0_per_point13=fov = .3;
wave_0_per_point14=
wave_0_per_point15=a = .05;
wave_0_per_point16=mod = (oz+1)*.5;
wave_0_per_point17=a = a*max(min(mod,1),0);
wave_0_per_point18=oz = oz - 2;
wave_0_per_point19=
wave_0_per_point20=ox = ox + t1;
wave_0_per_point21=oy = oy + t2;
wave_0_per_point22=
wave_0_per_point23=x = ox*fov/oz + 0.5;
wave_0_per_point24=x = (x-.5)*0.75 + 0.5;
wave_0_per_point25=y = oy*fov/oz + 0.5;
wave_0_per_point26=
wave_0_per_point27=r = 1;
wave_0_per_point28=g = .25+.25*sin(sp);
wave_0_per_point29=b = 0;
wave_0_per_point30=
wave_0_per_point31=minrgb = min(r,min(g,b));
wave_0_per_point32=maxrgb = max(r,max(g,b));
wave_0_per_point33=l = (maxrgb-minrgb)*.5;
wave_0_per_point34=diff = maxrgb-minrgb;
wave_0_per_point35=sum = maxrgb+minrgb;
wave_0_per_point36=s = if(above(l,0.5),diff/(2-sum),diff/sum)*(1-equal(l,0));
wave_0_per_point37=h = if(equal(r,maxrgb),(g-b)/diff,if(equal(g,maxrgb),2+(b-r)/diff,4+(r-g)/diff));
wave_0_per_point38=h = h*0.1666666;
wave_0_per_point39=h = if(below(h,0),0,if(above(h,1),1,h));
wave_0_per_point40=
wave_0_per_point41=h = h + time*0.05*1.324;
wave_0_per_point42=h = h - int(h);
wave_0_per_point43=
wave_0_per_point44=tmpb = if(below(l,0.5),l*(1+s),(l+s)-(s*l));
wave_0_per_point45=tmpa = 2*l - tmpb;
wave_0_per_point46=hvr = h + .333333;
wave_0_per_point47=hvr = if(below(hvr,0),hvr+1,if(above(hvr,1),hvr-1,hvr));
wave_0_per_point48=hvg = h;
wave_0_per_point49=hvg = if(below(hvg,0),hvg+1,if(above(hvg,1),hvg-1,hvg));
wave_0_per_point50=hvb = h - .333333;
wave_0_per_point51=hvb = if(below(hvb,0),hvb+1,if(above(hvb,1),hvb-1,hvb));
wave_0_per_point52=
wave_0_per_point53=r = if(below(6*hvr,1),tmpa+(tmpb-tmpa)*6*hvr, if(below(2*hvr,1),tmpb, if(below(hvr*3,2),tmpa+(tmpb-tmpa)*(.666666-hvr)*6,tmpa)));
wave_0_per_point54=g = if(below(6*hvg,1),tmpa+(tmpb-tmpa)*6*hvg, if(below(2*hvg,1),tmpb, if(below(hvg*3,2),tmpa+(tmpb-tmpa)*(.666666-hvg)*6,tmpa)));
wave_0_per_point55=b = if(below(6*hvb,1),tmpa+(tmpb-tmpa)*6*hvb, if(below(2*hvb,1),tmpb, if(below(hvb*3,2),tmpa+(tmpb-tmpa)*(.666666-hvb)*6,tmpa)));
wavecode_1_enabled=1
wavecode_1_samples=512
wavecode_1_sep=0
wavecode_1_bSpectrum=1
wavecode_1_bUseDots=0
wavecode_1_bDrawThick=1
wavecode_1_bAdditive=0
wavecode_1_scaling=1.0
wavecode_1_smoothing=0.5
wavecode_1_r=1.0
wavecode_1_g=1.0
wavecode_1_b=1.0
wavecode_1_a=1.0
wave_1_per_frame1=ps = if(above(time-tin,tr),1,0);
wave_1_per_frame2=tin = if(ps,time,tin);
wave_1_per_frame3=
wave_1_per_frame4=sz = 3.5;
wave_1_per_frame5=
wave_1_per_frame6=xr = if(ps,rand(sz*10*2 + 1)*.1 - sz,xr);
wave_1_per_frame7=yr = if(ps,rand(sz*10*2*.75 + 1)*.1 - sz*.75,yr);
wave_1_per_frame8=tr = if(ps,rand(11)*.1*.1,tr);
wave_1_per_frame9=
wave_1_per_frame10=sr = if(ps,(rand(8)+3)*.1*.6,sr);
wave_1_per_frame11=
wave_1_per_frame12=bob = if(ps,rand(q5)*100,bob);
wave_1_per_frame13=
wave_1_per_frame14=t1 = xr + bob;
wave_1_per_frame15=t2 = yr + bob;
wave_1_per_frame16=t3 = sr;
wave_1_per_point1=sp = sample*6.28*8*8*4;
wave_1_per_point2=it = it+1;
wave_1_per_point3=it = it*above(sample,0);
wave_1_per_point4=sz = t3;
wave_1_per_point5=ss = sample*6;
wave_1_per_point6=ox = sz*.5*pow(-1,it)*below(ss,1) + .5*pow(-1,it)*above(ss,1)*sz*below(ss,2) + above(ss,2)*.5*pow(-1,it)*sz*below(ss,3);
wave_1_per_point7=oy = (ss-.5)*sz*below(ss,1) + sz*.5*above(ss,1)*below(ss,2) + (.5-(ss-2))*sz*above(ss,2)*below(ss,3);
wave_1_per_point8=oz = -sz*.5*below(ss,1) + ((ss-1)-.5)*sz*above(ss,1)*below(ss,2) + sz*.5*above(ss,2)*below(ss,3);
wave_1_per_point9=ox = ox + above(ss,3)*below(ss,4)*-.5*sz + above(ss,4)*below(ss,5)*sz*(-.5+(ss-4)) + above(ss,5)*sz*.5;
wave_1_per_point10=oy = oy + above(ss,3)*below(ss,4)*.5*sz*pow(-1,it) + above(ss,4)*below(ss,5)*sz*-.5 + above(ss,5)*sz*(-.5+(ss-5));
wave_1_per_point11=oz = oz + above(ss,3)*below(ss,4)*sz*(.5-(ss-3)) + above(ss,4)*below(ss,5)*sz*.5*pow(-1,it) + above(ss,5)*sz*.5*pow(-1,it);
wave_1_per_point12=
wave_1_per_point13=fov = .3;
wave_1_per_point14=
wave_1_per_point15=a = .05;
wave_1_per_point16=mod = (oz+1)*.5;
wave_1_per_point17=a = a*max(min(mod,1),0);
wave_1_per_point18=oz = oz - 2;
wave_1_per_point19=
wave_1_per_point20=ox = ox + t1;
wave_1_per_point21=oy = oy + t2;
wave_1_per_point22=
wave_1_per_point23=x = ox*fov/oz + 0.5;
wave_1_per_point24=x = (x-.5)*0.75 + 0.5;
wave_1_per_point25=y = oy*fov/oz + 0.5;
wave_1_per_point26=
wave_1_per_point27=r = 1+sin(sp);
wave_1_per_point28=g = 0.5 + 0.5*sin(sample*1.57);
wave_1_per_point29=b = 0.5 + 0.5*cos(sample*1.57);
wavecode_2_enabled=1
wavecode_2_samples=512
wavecode_2_sep=0
wavecode_2_bSpectrum=1
wavecode_2_bUseDots=0
wavecode_2_bDrawThick=1
wavecode_2_bAdditive=0
wavecode_2_scaling=1.0
wavecode_2_smoothing=0.5
wavecode_2_r=1.0
wavecode_2_g=1.0
wavecode_2_b=1.0
wavecode_2_a=1.0
wave_2_per_frame1=ps = if(above(time-tin,tr),1,0);
wave_2_per_frame2=tin = if(ps,time,tin);
wave_2_per_frame3=
wave_2_per_frame4=sz = 3.5;
wave_2_per_frame5=
wave_2_per_frame6=xr = if(ps,rand(sz*10*2 + 1)*.1 - sz,xr);
wave_2_per_frame7=yr = if(ps,rand(sz*10*2*.75 + 1)*.1 - sz*.75,yr);
wave_2_per_frame8=tr = if(ps,rand(11)*.1*.1,tr);
wave_2_per_frame9=
wave_2_per_frame10=sr = if(ps,(rand(8)+3)*.1*.6,sr);
wave_2_per_frame11=
wave_2_per_frame12=bob = if(ps,rand(q5)*100,bob);
wave_2_per_frame13=
wave_2_per_frame14=t1 = xr + bob;
wave_2_per_frame15=t2 = yr + bob;
wave_2_per_frame16=t3 = sr;
wave_2_per_point1=sp = sample*6.28*8*8*4;
wave_2_per_point2=it = it+1;
wave_2_per_point3=it = it*above(sample,0);
wave_2_per_point4=sz = t3;
wave_2_per_point5=ss = sample*6;
wave_2_per_point6=ox = sz*.5*pow(-1,it)*below(ss,1) + .5*pow(-1,it)*above(ss,1)*sz*below(ss,2) + above(ss,2)*.5*pow(-1,it)*sz*below(ss,3);
wave_2_per_point7=oy = (ss-.5)*sz*below(ss,1) + sz*.5*above(ss,1)*below(ss,2) + (.5-(ss-2))*sz*above(ss,2)*below(ss,3);
wave_2_per_point8=oz = -sz*.5*below(ss,1) + ((ss-1)-.5)*sz*above(ss,1)*below(ss,2) + sz*.5*above(ss,2)*below(ss,3);
wave_2_per_point9=ox = ox + above(ss,3)*below(ss,4)*-.5*sz + above(ss,4)*below(ss,5)*sz*(-.5+(ss-4)) + above(ss,5)*sz*.5;
wave_2_per_point10=oy = oy + above(ss,3)*below(ss,4)*.5*sz*pow(-1,it) + above(ss,4)*below(ss,5)*sz*-.5 + above(ss,5)*sz*(-.5+(ss-5));
wave_2_per_point11=oz = oz + above(ss,3)*below(ss,4)*sz*(.5-(ss-3)) + above(ss,4)*below(ss,5)*sz*.5*pow(-1,it) + above(ss,5)*sz*.5*pow(-1,it);
wave_2_per_point12=
wave_2_per_point13=fov = .3;
wave_2_per_point14=
wave_2_per_point15=a = .05;
wave_2_per_point16=mod = (oz+1)*.5;
wave_2_per_point17=a = a*max(min(mod,1),0);
wave_2_per_point18=oz = oz - 2;
wave_2_per_point19=
wave_2_per_point20=ox = ox + t1;
wave_2_per_point21=oy = oy + t2;
wave_2_per_point22=
wave_2_per_point23=x = ox*fov/oz + 0.5;
wave_2_per_point24=x = (x-.5)*0.75 + 0.5;
wave_2_per_point25=y = oy*fov/oz + 0.5;
wave_2_per_point26=
wave_2_per_point27=r = 1+sin(sp);
wave_2_per_point28=g = 0.5 + 0.5*sin(sample*1.57);
wave_2_per_point29=b = 0.5 + 0.5*cos(sample*1.57);
wavecode_3_enabled=1
wavecode_3_samples=512
wavecode_3_sep=0
wavecode_3_bSpectrum=1
wavecode_3_bUseDots=0
wavecode_3_bDrawThick=1
wavecode_3_bAdditive=0
wavecode_3_scaling=1.0
wavecode_3_smoothing=0.5
wavecode_3_r=1.0
wavecode_3_g=1.0
wavecode_3_b=1.0
wavecode_3_a=1.0
wave_3_per_frame1=ps = if(above(time-tin,tr),1,0);
wave_3_per_frame2=tin = if(ps,time,tin);
wave_3_per_frame3=
wave_3_per_frame4=sz = 3.5;
wave_3_per_frame5=
wave_3_per_frame6=xr = if(ps,rand(sz*10*2 + 1)*.1 - sz,xr);
wave_3_per_frame7=yr = if(ps,rand(sz*10*2*.75 + 1)*.1 - sz*.75,yr);
wave_3_per_frame8=tr = if(ps,rand(11)*.1*.1,tr);
wave_3_per_frame9=
wave_3_per_frame10=sr = if(ps,(rand(8)+3)*.1*.6,sr);
wave_3_per_frame11=
wave_3_per_frame12=bob = if(ps,rand(q5)*100,bob);
wave_3_per_frame13=
wave_3_per_frame14=t1 = xr + bob;
wave_3_per_frame15=t2 = yr + bob;
wave_3_per_frame16=t3 = sr;
wave_3_per_point1=sp = sample*6.28*8*8*4;
wave_3_per_point2=it = it+1;
wave_3_per_point3=it = it*above(sample,0);
wave_3_per_point4=sz = t3;
wave_3_per_point5=ss = sample*6;
wave_3_per_point6=ox = sz*.5*pow(-1,it)*below(ss,1) + .5*pow(-1,it)*above(ss,1)*sz*below(ss,2) + above(ss,2)*.5*pow(-1,it)*sz*below(ss,3);
wave_3_per_point7=oy = (ss-.5)*sz*below(ss,1) + sz*.5*above(ss,1)*below(ss,2) + (.5-(ss-2))*sz*above(ss,2)*below(ss,3);
wave_3_per_point8=oz = -sz*.5*below(ss,1) + ((ss-1)-.5)*sz*above(ss,1)*below(ss,2) + sz*.5*above(ss,2)*below(ss,3);
wave_3_per_point9=ox = ox + above(ss,3)*below(ss,4)*-.5*sz + above(ss,4)*below(ss,5)*sz*(-.5+(ss-4)) + above(ss,5)*sz*.5;
wave_3_per_point10=oy = oy + above(ss,3)*below(ss,4)*.5*sz*pow(-1,it) + above(ss,4)*below(ss,5)*sz*-.5 + above(ss,5)*sz*(-.5+(ss-5));
wave_3_per_point11=oz = oz + above(ss,3)*below(ss,4)*sz*(.5-(ss-3)) + above(ss,4)*below(ss,5)*sz*.5*pow(-1,it) + above(ss,5)*sz*.5*pow(-1,it);
wave_3_per_point12=
wave_3_per_point13=fov = .3;
wave_3_per_point14=
wave_3_per_point15=a = .05;
wave_3_per_point16=mod = (oz+1)*.5;
wave_3_per_point17=a = a*max(min(mod,1),0);
wave_3_per_point18=oz = oz - 2;
wave_3_per_point19=
wave_3_per_point20=ox = ox + t1;
wave_3_per_point21=oy = oy + t2;
wave_3_per_point22=
wave_3_per_point23=x = ox*fov/oz + 0.5;
wave_3_per_point24=x = (x-.5)*0.75 + 0.5;
wave_3_per_point25=y = oy*fov/oz + 0.5;
wave_3_per_point26=
wave_3_per_point27=r = 1+sin(sp);
wave_3_per_point28=g = 0.5 + 0.5*sin(sample*1.57);
wave_3_per_point29=b = 0.5 + 0.5*cos(sample*1.57);
shapecode_0_enabled=0
shapecode_0_sides=4
shapecode_0_additive=0
shapecode_0_thickOutline=0
shapecode_0_textured=1
shapecode_0_x=0.2
shapecode_0_y=0.3
shapecode_0_rad=0.270481
shapecode_0_ang=0.0
shapecode_0_tex_ang=0.0
shapecode_0_tex_zoom=3.999140
shapecode_0_r=1.0
shapecode_0_g=1.0
shapecode_0_b=1.0
shapecode_0_a=1.0
shapecode_0_r2=1.0
shapecode_0_g2=1.0
shapecode_0_b2=1.0
shapecode_0_a2=1.0
shapecode_0_border_r=1.0
shapecode_0_border_g=1.0
shapecode_0_border_b=1.0
shapecode_0_border_a=0.1
shapecode_1_enabled=0
shapecode_1_sides=4
shapecode_1_additive=0
shapecode_1_thickOutline=0
shapecode_1_textured=0
shapecode_1_x=0.5
shapecode_1_y=0.5
shapecode_1_rad=0.1
shapecode_1_ang=0.0
shapecode_1_tex_ang=0.0
shapecode_1_tex_zoom=1.0
shapecode_1_r=1.0
shapecode_1_g=0.0
shapecode_1_b=0.0
shapecode_1_a=1.0
shapecode_1_r2=0.0
shapecode_1_g2=1.0
shapecode_1_b2=0.0
shapecode_1_a2=0.0
shapecode_1_border_r=1.0
shapecode_1_border_g=1.0
shapecode_1_border_b=1.0
shapecode_1_border_a=0.1
shapecode_2_enabled=0
shapecode_2_sides=4
shapecode_2_additive=0
shapecode_2_thickOutline=0
shapecode_2_textured=0
shapecode_2_x=0.5
shapecode_2_y=0.5
shapecode_2_rad=0.1
shapecode_2_ang=0.0
shapecode_2_tex_ang=0.0
shapecode_2_tex_zoom=1.0
shapecode_2_r=1.0
shapecode_2_g=0.0
shapecode_2_b=0.0
shapecode_2_a=1.0
shapecode_2_r2=0.0
shapecode_2_g2=1.0
shapecode_2_b2=0.0
shapecode_2_a2=0.0
shapecode_2_border_r=1.0
shapecode_2_border_g=1.0
shapecode_2_border_b=1.0
shapecode_2_border_a=0.1
shapecode_3_enabled=0
shapecode_3_sides=4
shapecode_3_additive=0
shapecode_3_thickOutline=0
shapecode_3_textured=0
shapecode_3_x=0.5
shapecode_3_y=0.5
shapecode_3_rad=0.1
shapecode_3_ang=0.0
shapecode_3_tex_ang=0.0
shapecode_3_tex_zoom=1.0
shapecode_3_r=1.0
shapecode_3_g=0.0
shapecode_3_b=0.0
shapecode_3_a=1.0
shapecode_3_r2=0.0
shapecode_3_g2=1.0
shapecode_3_b2=0.0
shapecode_3_a2=0.0
shapecode_3_border_r=1.0
shapecode_3_border_g=1.0
shapecode_3_border_b=1.0
shapecode_3_border_a=0.1
per_frame_1=wave_a = 0;
per_frame_2=
per_frame_3=sw = above(time-fin,0.06);
per_frame_4=fin = if(sw,time,fin);
per_frame_5=zoom = if(sw,.85,1);
per_frame_6=monitor = tic;
per_frame_7=
per_frame_8=vol = .1*(vol*9 + (bass_att+mid_att+treb_att)*.333333);
per_frame_9=q5 = 5 - vol*3.5;
per_frame_10=
warp_1=`shader_body
warp_2=`{
warp_3=`    // sample previous frame 3 times, creating radial blur
warp_4=`    float2 v = normalize(uv - 0.5)*aspect.xy;
warp_5=`    //v = v.yx * float2(1,-1);
warp_6=`    v *= texsize.zw*3;
warp_7=`    ret = 0.25*(   tex2D( sampler_main, uv ).xyz
warp_8=`                   +  tex2D(sampler_main, uv + v*2.5 ).xyz
warp_9=`                   +  tex2D(sampler_main, uv + v*5.5 ).xyz
warp_10=`                   +  tex2D(sampler_main, uv + v*-4  ).xyz
warp_11=`    );
warp_12=`    
warp_13=`    // darken over time
warp_14=`    ret -= 0.01;    
warp_15=`}
