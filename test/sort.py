#@author Gaurang Tandon
#For queries, contact author

#This code will automatically generate test tables and compare them with output table for equality

#run from tests directory
#the large input table is saved as../ data / test.csv
#output table is saved as ../data/sorted_test.csv

import itertools
import os
from random import randint, shuffle
from tqdm.auto import tqdm

DEBUG = False

def get_rand(start=1, end=50):
    return randint(start, end)


def test_table(
    input_table_name: str = "test",
    col_count: int = 5,
    sort_by_col_index: int = 2,
    row_count: int = 1000,
    sort_dir: str = "ASC", # or "DESC"
    bufsize: int = 10 # minimum 3
):
    saved_args = locals()
    if DEBUG:
        print("\n-------------\nRunning test", saved_args)
    sorted_col_values = [vall for vall in range(row_count)]
    shuffle(sorted_col_values)
    def get_row(row_index):
        def get_column_value(col_index):
            if col_index == sort_by_col_index:
                return sorted_col_values[row_index] - (row_count // 2)
            return get_rand(-100, 100)

        return [get_column_value(col_index) for col_index in range(col_count)]

    #first create input table
    input_table = [get_row(row_index) for row_index in range(row_count)]

    def get_col_char(index: int):
        return chr(ord("A") + index)

    with open(f"../data/{input_table_name}.csv", "w") as f:
        input_table_header = [get_col_char(x) for x in range(col_count)]
        f.write(", ".join(input_table_header) + "\n")
        for row in input_table:
            f.write(", ".join(map(str, row)) + "\n")

    #now create expected table
    expected_table = sorted(input_table, key=lambda row: row[sort_by_col_index], reverse=sort_dir == "DESC")
    sort_by_col_name = get_col_char(sort_by_col_index)

    #write the query
    output_table_name = f"sorted_test"
    output_table_path = f"../data/{output_table_name}.csv"

    query_text = f"LOAD {input_table_name}\n{output_table_name} <- SORT {input_table_name} BY {sort_by_col_name} IN {sort_dir} BUFFER {bufsize}\nEXPORT {output_table_name}"
    if DEBUG:
        print("Running query\n", query_text)
    query_file_path = f"../data/Q.txt"
    with open(query_file_path, "w") as f:
        f.write(query_text)

    #run the query
    os.system(f"../src/server < {query_file_path} > dump")

    #read output table
    output_table = []
    with open(output_table_path) as f:
        for line in f.readlines()[1:]:
            row = list(map(int, line.split(",")))
            output_table.append(row)

    #compare to expected table
    got_size = len(output_table)
    exp_size = len(expected_table)
    assert got_size == exp_size, f"Sizes don't match: got {got_size} vs required {exp_size}"

    for rowIndex in range(got_size):
        row_got = output_table[rowIndex]
        row_exp = expected_table[rowIndex]
        row_len_got = len(row_got)
        row_len_exp = len(row_exp)
        assert row_len_exp == row_len_got, f"Row lengths don't match: got {row_len_got} ({row_got}) vs required {row_len_exp} ({row_exp})"

        assert all(row_got[i] == row_exp[i] for i in range(row_len_got)), f"Rows don't match: got {row_got} vs required {row_exp}"


if __name__ == "__main__":
    os.system(f"cd ../src; make; cd ../tests")
    row_range = list(range(1, 100)) + [1005, 5017]
    bufrange = list(range(3, 20)) + [100, 1000, 1500]
    dirns = ["ASC", "DESC"]
    all_combos = itertools.product(row_range, bufrange, dirns)

    for row_count, bufsize, direction in tqdm(list(all_combos), desc="Running tests"):
        test_table(sort_dir=direction, bufsize=bufsize, sort_by_col_index=1, row_count=row_count)
    print("All tests passed")
