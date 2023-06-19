# libuaic

**uAIC** is a small machine learning library for C.

## Features

- Designed to be easy to use and integrate into your C projects.
- Provides a range of functionalities for data preprocessing, model training, and prediction.
- Supports supervised learning tasks.
- Offers high-performance capabilities to handle large datasets.
- Includes comprehensive documentation and examples to help you get started quickly.

## Installation

To use **uAIC** in your C projects, follow these steps:



1. Clone the repository: `git clone https://github.com/CodenameSource/libuaic.git`
2. Navigate to the project directory: `cd libuaic`
3. Compile the library: `make lib`
4. Link the library with your project: `gcc -o myproject myproject.c -I./include -L./lib -luaic`
5. Run your project: `./myproject`

## Usage

Once you have linked the library with your project, you can start using **uAIC**. Here's a simple example to demonstrate how to train a linear regression model:

```c
int main()
{
    DataFrame test = {0}, X = {0}, Y = {0};
    UAI_MUST(df_load_csv(&test, "csv/houses.csv", ','));
    df_set_header(&test, true);

    df_to_double(&test, DATACELL_CONVERT_STRICT);
    df_normalize(&test);

    srand(time(NULL));

    UAI_MUST(df_create_vsplit(&test, &Y, 1, DATAFRAME_SAMPLE_SEQ));
    UAI_MUST(df_create_vsplit(&test, &X, 4, DATAFRAME_SAMPLE_SEQ));

    LinearRegressor *reg = lr_init();
    lr_fit(reg, &X, &Y, 2000, 0.015);

    for (size_t r=0; r < Y.rows; ++r)
        printf("%lf\n", uai_denormalize_value(lr_predict(reg, X.data[r], X.cols), Y.data[0][0].min, Y.data[0][0].delta));

    lr_destroy(reg);

    df_destroy(&test);
    df_destroy(&X);
    df_destroy(&Y);
}
```

For more detailed information on the available functionalities and how to use them, refer to the [documentation](https://github.com/CodenameSource/libuaic/wiki).

### Example code

Extensive example code is provided in the [`examples/`](./examples/) directory. You can build and run the examples using:

```console
$ make
```

All the binaries will be built in `./examples/*.out`.

The following examples are available:

- `dataframe/load_csv`
- `dataframe/convert`
- `dataframe/export_csv`
- `dataframe/split`
- `dataframe/fill`
- `dataframe/resize`
- `dataframe/scale_data`
- `logistic_regression/logistic_regression`
- `classification/decision_tree`
- `linear_regression/linear_regression`

## Contributing

Contributions are welcome! If you would like to contribute to **uAIC**, please follow the guidelines outlined in [CONTRIBUTING.md](./CONTRIBUTING.md).

## License

This project is licensed under the [GPL License](./LICENSE).
