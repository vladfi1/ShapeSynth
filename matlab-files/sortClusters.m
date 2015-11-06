function [clustCentSorted, point2clusterSorted] = sortClusters(clustCent, point2cluster,nofClusters)

point2clusterSorted = zeros(size(point2cluster));
clustCentSorted = zeros(size(clustCent));

max_label = max(point2cluster);
min_label = min(point2cluster);
clusterPopulation = zeros(nofClusters,1);
labels = zeros(nofClusters,1);
j=1;

for i=min_label:max_label
clusterPopulation(j) = numel(find(point2cluster == i)); 
labels(j) = i;
j = j + 1;
end

[sortedPop, indx] = sort(clusterPopulation, 'descend');
labels = labels(indx);

for i=1:size(clustCentSorted,1)
    clustCentSorted(i,:) = clustCent(indx(i),:);
end

for i=1:size(point2cluster,1)
point2clusterSorted(i) = find(labels == point2cluster(i));
end 