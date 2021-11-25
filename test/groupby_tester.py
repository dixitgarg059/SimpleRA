# @author Gaurang Tandon
# For queries, contact author

# This code will automatically generate test tables and compare them with output table for equality

# run from tests directory
# the large input table is saved as ../data/test.csv
# query test files are saved as ../data/QMIN.txt
# output table files are exported as ../data/TMIN.txt

from collections import defaultdict
import os
from random import randint


def get_rand(start=1, end=50):
    return randint(start, end)


def test_table(
    input_table_name: str = "test",
    col_count: int = 5,
    group_by_col_index: int = 2,
    row_count: int = 1000,
    no_groupby_special: bool = False,
):
    def get_row():
        def get_column_value(col_index):
            if no_groupby_special or col_index != group_by_col_index:
                return get_rand()
            # smaller range for group by column so as to make groups nicer
            return get_rand(20, 25)

        return list(
            map(
                str,
                [get_column_value(col_index) for col_index in range(col_count)],
            )
        )

    # first create input table
    input_table = [get_row() for _ in range(row_count)]

    def get_col_char(index: int):
        return chr(ord("A") + index)

    with open(f"../data/{input_table_name}.csv", "w") as f:
        input_table_header = [get_col_char(x) for x in range(col_count)]
        f.write(", ".join(input_table_header) + "\n")
        for row in input_table:
            f.write(", ".join(row) + "\n")

    # now create expected outputs
    test_functions = {
        "MIN": min,
        "MAX": max,
        "AVG": lambda x: sum(x) // len(x),
        "SUM": sum,
    }

    def get_other_col():
        col = get_rand(start=0, end=col_count - 1)
        return col if col != group_by_col_index else get_other_col()

    group_by_col = get_col_char(group_by_col_index)
    for funcname, func in test_functions.items():
        other_col_index = get_other_col()
        other_col = get_col_char(other_col_index)

        # write the query
        output_table_name = f"T{funcname}"
        output_table_path = f"../data/{output_table_name}.csv"

        query_text = f"LOAD {input_table_name}\n{output_table_name} <- GROUP BY {group_by_col} FROM {input_table_name} RETURN {funcname}({other_col})\nEXPORT {output_table_name}"
        query_file_path = f"../data/Q{funcname}.txt"
        with open(query_file_path, "w") as f:
            f.write(query_text)

        # run the query
        os.system(f"../src/server < {query_file_path}")

        # read output table
        output_table = {}
        with open(output_table_path) as f:
            for line in f.readlines()[1:]:
                key, value = map(int, line.split(","))
                assert key not in output_table, f"Duplicate key {key} found"
                output_table[key] = value

        # build expected table
        expected_table_vals = defaultdict(list)
        for row in input_table:
            gb_val = int(row[group_by_col_index])
            other_val = int(row[other_col_index])
            expected_table_vals[gb_val].append(other_val)
        expected_table = {k: func(v) for k, v in expected_table_vals.items()}

        # compare to expected table
        for k, v in expected_table.items():
            assert k in output_table, f"Missing key {k}"
            assert (
                output_table[k] == v
            ), f"Key {k} mismatch. expected: {v}, actual: {output_table[k]}"
            del output_table[k]
        assert (
            len(output_table) == 0
        ), "Output table has extra keys: {list(output_table.keys())}"


if __name__ == "__main__":
    test_table(no_groupby_special=False)
    test_table(no_groupby_special=True)
