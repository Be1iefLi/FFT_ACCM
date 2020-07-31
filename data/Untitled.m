for i = 3:499
    filename = sprintf('%d.txt',i);
    file = fopen(filename);
    c = textscan(file,'%f');
    fclose(file);
    c = c{1,1};
    c = c * 8192;
    c = int32(c);
    format = '%d\n';
    file = fopen(filename,'w');
    fprintf(file, format, c);
    fclose(file);
end