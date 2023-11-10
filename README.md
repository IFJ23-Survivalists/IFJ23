# IFJ23 - Swift Interpreter

IFJ23 is a school project aimed at implementing an interpreter for a simplified version of the Swift programming language. This project is part of our coursework for [Brno University
of Technology](https://www.vut.cz/en/) in [IFJ](https://www.fit.vut.cz/study/course/IFJ/.en).
## Table of Contents

- [Project Overview](#project-overview)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Code Guidelines](#code-guidelines)
- [Authors](#authors)
- [License](#license)

## Project Overview

The IFJ23 project is designed to implement a Swift interpreter that can handle a simplified version of the Swift programming language. The project is divided into several key components:

1. **Lexical Analysis**: Responsible for tokenizing the input source code.
2. **Syntax Analysis**: Parses the tokenized code into a syntax tree.
3. **Semantic Analysis**: Performs semantic checks and builds symbol tables.
4. **Internal Code Generation**: Generates intermediate code from the syntax tree.
5. **Optimization**: Optimizes the intermediate code for better performance.
6. **Code Generation**: Produces executable code from the optimized intermediate code.

## Getting Started

To get started with the IFJ23 project, follow these steps:

1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/IFJ23-Survivalists/IFJ23
   cd IFJ23
   ```

2. Build the project using the make command:
    ```bash
    make
    ```
3. Execute the Swift interpreter with your Swift source code:
    ```bash
    ./ifj23 your_swift_file.swift
    ```

## Usage
Given the follow `factorial.swift` that calculates factorial.

```swift
write("Zadejte cislo pro vypocet faktorialu\n")
let a : Int? = readInt()
if let a {
  if (a < 0) {
    write("Faktorial nelze spocitat\n")
  } else {
    var a = Int2Double(a)
    var vysl : Double = 1
    while (a > 0) {
      vysl = vysl * a
      a = a - 1
    }
    write("Vysledek je: ", vysl, "\n")
  }
} else {
  write("Chyba pri nacitani celeho cisla!\n")
}
```

Execute it simply by
```bash
./ifj23 factorial.swift
```

## Code Guidelines

Please IFJ23 project, please follow these code guidelines:

- **Documentation**: Every file must be documented at the start using Doxygen format, including the `@file`, `@date`, `@authors`, and `@brief` tags. For example:

    ```c
    /**
     * @file your_file.c
     * @date dd/mm/yyyy the last edited day
     * @authors author1
     *          author2
     * @brief A file that is documented.
     */
    ```

- **Header Declarations**: Document every declaration in the header file (enum, union, struct, function, constants) using Doxygen style. Include `@brief` for each declaration at minimum. For example:

    ```c
    #ifndef EXAMPLE_H
    #define EXAMPLE_H

    /**
     * @brief Short description about the function
     *
     * Detailed description about the function.
     * @param arg1 The first argument
     * @param arg2 The second argument
     * @return Return something
     */
    double do_something(char arg1, int arg2);

    #endif
    ```

- **Function Naming**: Functions that are methods over a data structure must start with the data structure name and have a pointer to that data structure as the first parameter if needed. For example:

    ```c
    typedef struct { ... } String;

    void string_init(String *str);
    void string_push(String *str, char ch);
    ```

- **Data Types**: All data types in header files should be declared with `typedef`.

- **Naming Convention**:
    - *Variables*: Use `snake_case` for variable names. For example:

        ```c
        int main() {
            int just_for_demonstration = 0;
            return just_for_demonstration;
        }
        ```

    - *Global constants*: Use `UPPERCASE` for global constants. For example:

        ```c
        const double EULER_NUMBER = 2.7;
        ```
        
    - *Global variables*: Use prefix `g_` for global variables. For example:
      
      ```c
      int g_error = 0;
      ```

    - *Static variables*: Use prefix `s_` for global static variables. For example:

      ```c
      int foo() {
         static int s_num = 0;
      }
      ```
      
    - *Functions*: Use `snake_case` for function names. For example:

        ```c
        void my_beautiful_function();
        ```

    - *Typedef*: Use `PascalCase` for `typedef` declarations. For example:

        ```c
        typedef struct { ... } TheUltimateStruct;
        ```

    - *Enum Items*: Prefix them with the enum name, followed by an underscore, and then use `PascalCase`. For example:

        ```c
        typedef enum {
            DataType_Number,
            DataType_FloatingPointNumber,
            DataType_String,
        } DataType;
        ```

- **Error Handling**: Use the provided `set_error` and `got_error` functions declared in the `error.h` file for error handling. If you are unsure which function is failable, perform error handling after calling any function. For example:

    ```c
    void my_failable_function() {
        if (true) {
            eprint("Something went wrong\n");
            set_error(Error_Internal);
            return;
        }
    }

    void other_function() {
        my_failable_function();

        if (got_error()) {
            // Perform error handling here, usually early exit
            return;
        }
    }
    ```

## Authors
Our team consists of four dedicated members:
- [xkloub03 - Jakub Kloub](https://github.com/TheRetikGM)
- [xmorav48 - Matúš Moravčík](https://github.com/Blazeo7)
- [xnguye27 - Le Duy Nguyen](https://github.com/tmokenc)
- [xhabrl01 - Lukáš Habr](https://github.com/LukasHabr)
## License
This program is licensed under the GPL-3.0 License - see the LICENSE file for details.
