function theta = mradon(img)
%UNTITLED 此处显示有关此函数的摘要
%   此处显示详细说明
%bg = rgb2gray(img);

theta=1:179;
[R,xp]=radon(img,theta);
[I0,J]=find(R>=max(R(:)));%J记录了倾斜角
if J<=90
   qingxiejiao=J;
else
   qingxiejiao=J-180;
end

theta = - qingxiejiao;

end

