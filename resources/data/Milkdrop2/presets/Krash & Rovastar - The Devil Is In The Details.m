[preset00]
fRating=3.000000
fGammaAdj=2.993998
fDecay=1.000000
fVideoEchoZoom=1.000000
fVideoEchoAlpha=0.500000
nVideoEchoOrientation=1
nWaveMode=0
bAdditiveWaves=0
bWaveDots=0
bWaveThick=0
bModWaveAlphaByVolume=0
bMaximizeWaveColor=1
bTexWrap=0
bDarkenCenter=0
bRedBlueStereo=0
bBrighten=0
bDarken=0
bSolarize=0
bInvert=0
fWaveAlpha=1.000000
fWaveScale=0.334693
fWaveSmoothing=0.750000
fWaveParam=-0.219900
fModWaveAlphaStart=0.750000
fModWaveAlphaEnd=0.950000
fWarpAnimSpeed=1.000000
fWarpScale=1.000000
fZoomExponent=1.000000
fShader=1.000000
zoom=0.999900
rot=0.100000
cx=0.500000
cy=0.500000
dx=0.000000
dy=0.000000
warp=1.000000
sx=1.000000
sy=1.000000
wave_r=0.500000
wave_g=0.500000
wave_b=0.500000
wave_x=0.500000
wave_y=0.500000
ob_size=0.010000
ob_r=0.000000
ob_g=0.000000
ob_b=0.000000
ob_a=1.000000
ib_size=0.005000
ib_r=0.400000
ib_g=0.000000
ib_b=0.000000
ib_a=1.000000
nMotionVectorsX=64.000000
nMotionVectorsY=48.000000
mv_dx=0.000000
mv_dy=0.000000
mv_l=0.000000
mv_r=0.000000
mv_g=0.700000
mv_b=1.000000
mv_a=0.000000
per_frame_1=warp=0;
per_frame_2=wave_r = wave_r + 0.45*(0.5*sin(time*0.701)+ 0.3*cos(time*0.438));
per_frame_3=wave_b = wave_b - 0.4*(0.5*sin(time*4.782)+0.5*cos(time*0.722));
per_frame_4=wave_g = wave_g + 0.4*sin(time*1.931);
per_frame_5=vol = 0.167*(bass+mid);
per_frame_6=xamptarg = if(equal(frame%15,0),min(0.5*vol*bass_att,0.5),xamptarg);
per_frame_7=xamp = xamp + 0.5*(xamptarg-xamp);
per_frame_8=xdir = if(above(abs(xpos),xamp),-sign(xpos),if(below(abs(xspeed),0.1),2*above(xpos,0)-1,xdir));
per_frame_9=xspeed = xspeed + xdir*xamp - xpos - xspeed*0.055*below(abs(xpos),xamp);
per_frame_10=xpos = xpos + 0.001*xspeed;
per_frame_11=wave_x = 1.25*xpos + 0.5;
per_frame_12=yamptarg = if(equal(frame%15,0),min(0.3*vol*treb_att,0.5),yamptarg);
per_frame_13=yamp = yamp + 0.5*(yamptarg-yamp);
per_frame_14=ydir = if(above(abs(ypos),yamp),-sign(ypos),if(below(abs(yspeed),0.1),2*above(ypos,0)-1,ydir));
per_frame_15=yspeed = yspeed + ydir*yamp - ypos - yspeed*0.055*below(abs(ypos),yamp);
per_frame_16=ypos = ypos + 0.001*yspeed;
per_frame_17=wave_y = 1.25*ypos + 0.5;
per_frame_18=q2=1.1*xpos +0.25*ypos + 0.5;
per_frame_19=q1=1.1*ypos +0.25*xpos + 0.5;
per_frame_20=ib_r = 0.3+xpos;
per_frame_21=ib_b = 0.06*bass;
per_frame_22=ib_g = 0.25+ypos;
per_frame_23=q3 = 10+8*(0.6*sin(0.423*time) + 0.4*sin(0.253*time));
per_frame_24=q4 = 1/q3;
per_frame_25=q5 = 0.5*sign(xpos);
per_frame_26=q6 = 0.5*sign(ypos);
per_frame_27=monitor = rot;
per_pixel_1=cx = ((0&(x*q3-q5))+q5)*q4;
per_pixel_2=cy = ((0&(y*q3-q6))+q6)*q4;
per_pixel_3=newx = q1-x;
per_pixel_4=newy = q2-y;
per_pixel_5=newrad = sqrt((newx)*(newx)+0.5625*(newy)*(newy))*2;
per_pixel_6=newzoom = pow(1.05 + 0.03*newrad, pow(0.01+sin(newrad*newrad), newrad*2-1));
per_pixel_7=dx = (newx)*newzoom - newx;
per_pixel_8=dy = (newy)*newzoom - newy;
per_pixel_9=dx =dx*0.1;
per_pixel_10=dy=dy*0.1;
per_pixel_11=rot = 2*newrad*(0.5*(0.5-rad)+0.1);
