
start=0, end = 2;
for (index = start; index < start +(end-start)/2; index++) {
    temp = arr[index];
    arr[index] = arr[end-1-index];
    arr[end-1-index] = temp;

}