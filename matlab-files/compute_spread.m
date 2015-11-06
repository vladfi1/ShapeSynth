function [spread] = compute_spread(input_points, fraction)

diss = pdist(input_points); 
[h, x] = hist(diss);
threshold = fraction*sum(h);

index = 2; % default index
for i=1:size(h,2)
    if h(i)<threshold
        index = i;
        break;
    end
end

spread = x(index);  %% auto threshold choice
%%% spread is not subsequently used here!