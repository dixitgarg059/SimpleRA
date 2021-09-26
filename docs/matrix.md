## Phase 0
### Dixit Kumar Garg(2018101077), Manish(2018101073)
### Overview
- As Matrix and Table have very different properties so we have created a new matrix class.
- To support the matrix commands syntactic and semantic parsers were modified.
- Matrix Catalogue class was also created which has functionalities similar to the table catalogue.
- Export and Print functionalities were also modified to support our storage design.

### Load (Page Details)
- The matrix is stored in pages in form of tuples containing only 1 column, the value of the element.
- The matrix of size n\*n is divided into pages. Each page contains a square submatrix of the n\*n matrix. If the page size is p then the side of submatrix will be s such that s\*s = p. The elements of the submatrix are flattened into a 1-d array and stored in the page contigously. 
- To make sure there are not more than 2 pages in the memory at a time we read the csv file word by word. Then the data is blockified into pages and stored in the disk.
- Each tuple contains 1 integer and their will be n\*n tuples. Hence occupying a total space of n\*n\*4 bytes. 
- In total n\*n/p pages of page size p are required to store a matrix of sie n\*n.

### Print/Export
- As both Print and Export are very similar we will cover them jointly.
- To print the matrix we iterate through the the pages and print row by row. We load the first page print its first row, load second page and print first row etc. After we are done with printing the first row we move on the second row and go through the pages again. 
- If the submatrix side length is s then there are s*n page reads. 

### Transpose
- For each submatrix/page there is a complementary submatrix such that for every element (i,j) in the submatrix, (j,i) will lie in the complementary submatrix. To find transpose for this matrix we swap the the elemepage with its complementary page and then find transpose for each page independently. This way at any moment only 2 pages will be brought in the memory.
- Time complexity of transpose operation will be O(n*n). All pages are brought in the memory, swapped with another page and all pages are brought into the memory only once.
- This is an inplace operation as we are just swapping the pages.

## Sparse Matrix
- For sparse matrix we only store non zero values. 
- These values are stored in the form of tuples (row, column, val). These tuples are stored in sorted order in the pages. We stored these pages using the existing table storage. 


### Load (Page Details)
- The matrix is stored in pages in form of tuples (row, column, val).
- All these tuples are stored in a sorted manner such that the tuple with lower row value comes first and if both of them have same values than the one with lower column value comes first.
- To make sure there are not more than 2 pages in the memory at a time we read the csv file word by word. Then the data is blockified into pages and stored in the disk.
- Number of tuples will be equal to the number of non zero values in the matrix. Hence occupying a total space of n\*n\*12 bytes. 

### Print/Export
- As both Print and Export are very similar we will cover them jointly.
- After storing the tuples in sorted order in the pages. Printing becomes trivial as start iterating from row=0 and column=0. The rows and columns of the first tuple in the first page is initially set as the target. Now we keep on printing 0 untill we reach the target row and target column. At that point we print the target value and increase our target iterator to the next tuple.


### Transpose
- This compression technique not only reduce the space occupied by the matrix but also makes the transpose operation very easy. 
- As we are storing tuples of (row, column, val) transposing the matrix just means swapping the column and row column. So for transposing we rename the row as column and column as row.
- But an issue arises when we try to print after transposing the matrix. As the row and columns are swapped in the transpose operation. The tuples are no longer sorted with respect to rows. They are sorted with request to the column and hence we can not use the same print method as earlier. Therefore we sort the tuples with respect to the row. 
- Sorting the tuples involve sorting each of the page indivisually and then merging the sorted pages. 
- Sorting is also inplace. We first bring i'th page in the memory. Now for every j in (i+1, n) we bring the j'th page in the memory and and merge the 2 pages. Among the elements of both i'th and j'th page smallest n elements are left in the i'th page and the largest n elements are placed in the j'th page. This operation is repeated for each page from (0, n). After completing this iteration the tuples will be again in the sorted order. 


### Compression Ratio

- Let the dimensions of the matrix be n\*n.
- Let the ratio of zero values in the matrix be z. 
- Then there will be (1-z)\*n\*n non zero values. 
- Hence total space occupied by the compressed sparse matrix will be (1-z)\*n\*n\*12 bytes. 
- Space occupied by the dense matrix of dimension n\*n will be n\*n\*4. 
- Compression ratio c = 1/(3-3\*z), which is the ratio of space occupied by the sparse and matrix and the dense matrix.
- Examples (z>=.6)
  For z = .6, c = .83, so dense storage will be more space efficient in this case. But using sparse stoarage other operations are also becoming faster.
  For z = .66, c = 1, so dense storage will require same space as the sparse storage.
  For z = .8, c = 1.3, so sparse storage will require less space than the dense storage and this ratio keeps on increasing as z increases.
  For z = 1, c = 33.33, actually in this case none of the values are non zero so the sparse storage will not require any storage space. 
