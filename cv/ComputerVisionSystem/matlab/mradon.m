function theta = mradon(img)
%UNTITLED �˴���ʾ�йش˺�����ժҪ
%   �˴���ʾ��ϸ˵��
%bg = rgb2gray(img);

theta=1:179;
[R,xp]=radon(img,theta);
[I0,J]=find(R>=max(R(:)));%J��¼����б��
if J<=90
   qingxiejiao=J;
else
   qingxiejiao=J-180;
end

theta = - qingxiejiao;

end

