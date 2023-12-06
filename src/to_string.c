/**
 * @note Project: Implementace překladače imperativního jazyka IFJ23
 * @brief Implementation for to_string.h
 * @file to_string.c
 * @author Jakub Kloub, xkloub03, VUT FIT
 * @date 28/11/2023
 */
#include "to_string.h"
#include <math.h>

const char* token_to_string(const Token* tok) {
    switch (tok->type) {
        case Token_Whitespace:
            if (tok->attribute.has_eol)
                return "EOL";
            return TOKENTYPE_NAMES[tok->type];
        case Token_Data:
            switch (tok->attribute.data.type) {
                case DataType_Int:
                case DataType_Double:
                case DataType_MaybeInt:
                case DataType_MaybeDouble:
                    return "Numeric constant";
                case DataType_String:
                case DataType_MaybeString:
                    return "String literal";
                case DataType_Bool:
                case DataType_MaybeBool:
                    return "Boolean value";
                case DataType_Undefined:
                    if (tok->attribute.data.is_nil)
                        return "nil";
                    return "Undefined value";
                default:
                    MASSERT(false, "token_to_string: Unknown type of attribute.data");
                    break;
            }
            break;
        case Token_DataType:
            return datatype_to_string(tok->attribute.data_type);
        case Token_Operator:
            return operator_to_string(tok->attribute.op);
        case Token_Identifier:
            return tok->attribute.data.value.string.data;
        default:
            return tokentype_to_string(tok->type);
    }
}

String unsigned_to_string(unsigned num) {
    int num_digits = num != 0 ? (int)(log10(num)) + 1 : 1;

    String str;
    string_init(&str);
    string_reserve(&str, num_digits + 1);
    str.length = num_digits;
    str.data[num_digits] = '\0';
    sprintf(str.data, "%i", num);
    return str;
}
