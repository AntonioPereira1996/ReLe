function phi = dam_basis_rbf(state)

n_centers = 4;
if nargin == 0
    phi = basis_rbf(n_centers,[0,160]) + 1;
else
    phi = [1; basis_rbf(n_centers,[0,160],state)];
end
    
% nr_centers = 4;
% centers = [0; 50; 120; 160];
% widths = [50; 20; 40; 50];
% 
% if nargin == 0
%     phi = nr_centers + 1;
% else
%     phi = zeros((size(centers, 1) + 1), 1);
%     
%     index = 1;
%     phi(index) = 1;
%     for i = 1 : size(centers, 1)
%         phi(index + i) = exp(-norm(state - centers(i)) / widths(i));
%     end
% end

return;
