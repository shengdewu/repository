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
%imagesc(theta, xp, R); %������R�е�Ԫ����ֵ����Сת��Ϊ��ͬ��ɫ�������������Ӧλ�ô���������ɫȾɫ
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
figure(8),subplot(3,1,1),imshow(bImg),title('1.��λ������ƻҶ�ͼ��');
figure(8),subplot(3,1,2),imshow(pic),title('2.����radon������ˮƽ�������');

break;
%����radon�任����ֱ����Ľ���
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
%�Ƕȼ������
H=[1,0,0; tan(-theta_max),1,0;0,0,1];
T=maketform('affine',H);
pic=imtransform(bImg,T);
figure(8),subplot(3,1,3), imshow(bImg);title('3.����radon��������ֱ�������');


