im = imread('F:\IdentificationCodes\IdentificationCodes\source\sample\10 (6).bmp');
im = rgb2gray(im);
theta = mradon_1(im);
I1=imrotate(im,theta);
subplot(1,2,1),imshow(im);title('Original image');
subplot(1,2,2),imshow(I1);title('correct image');




