clear all;
clc;

im = imread('F:\IdentificationCodes\IdentificationCodes\matlab.bmp');
g = rgb2gray(im);
l = 171/ 255;
bImg = imcomplement(im2bw(uint8(g), l));

%%edge detection and edge linking...
bImg = edge(bImg, 'sobel');
%bImg = bwmorph(bImg, 'thicken');

%%Radon transform projections along 180 degress, from -90 to +89
theta = -89 : 89;
[R, xp] = radon(bImg, theta)
%imagesc(theta, xp, R); %将矩阵R中的元素数值按大小转化为不同颜色，并在坐标轴对应位置处以这种颜色染色
%colormap(jet);
%xlabel('theta (degress)');
%ylabel('x');
%colorbar;
[m, n] = size(R);
c = 1;
for i=1:m
    for j=1:n
        R(1,1) = R(i,j);
        c = j;
    end
end

rot = 90-c+2;
rot = -50;
pic = imrotate(bImg, rot, 'crop');
figure(8),subplot(3,1,1),imshow(bImg),title('1.定位后的这牌灰度图像');
figure(8),subplot(3,1,2),imshow(pic),title('2.利用radon算子做水平方向矫正');

break;
%利用radon变换做垂直方向的矫正
binaryImage = edge(bImg,'canny'); 
binaryImage = bwmorph(binaryImage,'thicken'); 
theta = -90:89;
[R,xp] = radon(binaryImage,theta);
%%%%%
[R1,r_max] = max(R);
theta_max = 90;
while (theta_max > 50 || theta_max<-50)
    [R2,theta_max] = max(R1);                      
    R1(theta_max) = 0; 
    theta_max = theta_max - 91;
end
%角度计算完毕
H=[1,0,0; tan(-theta_max),1,0;0,0,1];
T=maketform('affine',H);
pic=imtransform(bImg,T);
figure(8),subplot(3,1,3), imshow(bImg);title('3.利用radon算子做垂直方向矫正');


