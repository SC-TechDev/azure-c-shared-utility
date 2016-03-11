// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>

#include "umock_c.h"
#include "umockcall.h"
#include "umocktypes.h"
#include "umocktypes_stdint.h"

static size_t expected_call_count;
static UMOCKCALL_HANDLE* expected_calls;
static size_t actual_call_count;
static UMOCKCALL_HANDLE* actual_calls;
static char* expected_calls_string;
static char* actual_calls_string;

int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error)
{
    int result;
    (void)on_umock_c_error;
    if ((umocktypes_init() != 0) ||
        (umocktypes_stdint_register_types() != 0))
    {
        result = __LINE__;
    }
    else 
    {
        result = 0;
    }

    return result;
}

void umock_c_deinit(void)
{
    umock_c_reset_all_calls();

    free(actual_calls_string);
    actual_calls_string = NULL;
    free(expected_calls_string);
    expected_calls_string = NULL;
    umocktypes_deinit();
}

int umock_c_reset_all_calls(void)
{
	if (expected_calls != NULL)
	{
        size_t i;
        for (i = 0; i < expected_call_count; i++)
        {
            umockcall_destroy(expected_calls[i]);
        }

        free(expected_calls);
		expected_calls = NULL;
	}
	expected_call_count = 0;

    if (actual_calls != NULL)
    {
        size_t i;
        for (i = 0; i < actual_call_count; i++)
        {
            umockcall_destroy(actual_calls[i]);
        }

        free(actual_calls);
        actual_calls = NULL;
    }
    actual_call_count = 0;

	return 0;
}

int umock_c_add_expected_call(UMOCKCALL_HANDLE mock_call)
{
    int result;
    UMOCKCALL_HANDLE* new_expected_calls = (UMOCKCALL_HANDLE*)realloc(expected_calls, sizeof(UMOCKCALL_HANDLE) * (expected_call_count + 1));
	if (new_expected_calls == NULL)
	{
		result = __LINE__;
	}
	else
	{
		expected_calls = new_expected_calls;
        expected_calls[expected_call_count++] = mock_call;
        result = 0;
	}

    return result;
}

UMOCKCALL_HANDLE umock_c_add_actual_call(UMOCKCALL_HANDLE mock_call)
{
    UMOCKCALL_HANDLE result = NULL;
    size_t i;

    /* Codes_SRS_UMOCK_C_01_115: [ umock_c shall compare calls in order. ]*/
    for (i = 0; i < expected_call_count; i++)
    {
        if (umockcall_are_equal(expected_calls[i], mock_call))
        {
            result = expected_calls[i];
            (void)memmove(&expected_calls[i], &expected_calls[i + 1], (expected_call_count - i - 1) * sizeof(UMOCKCALL_HANDLE));
            expected_call_count--;
            i--;
            break;
        }
    }

    if (i == expected_call_count)
    {
        /* an unexpected call */
        UMOCKCALL_HANDLE* new_actual_calls = (UMOCKCALL_HANDLE*)realloc(actual_calls, sizeof(UMOCKCALL_HANDLE) * (actual_call_count + 1));
        if (new_actual_calls == NULL)
        {
            result = NULL;
        }
        else
        {
            actual_calls = new_actual_calls;
            actual_calls[actual_call_count++] = mock_call;
            result = NULL;
        }
    }
    else
    {
        umockcall_destroy(mock_call);
    }

    return result;
}

const char* umock_c_get_expected_calls(void)
{
    const char* result;
    size_t i;
    char* new_expected_calls_string;

    new_expected_calls_string = (char*)realloc(expected_calls_string, 1);
    if (new_expected_calls_string == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t current_length = 0;
        expected_calls_string = new_expected_calls_string;
        expected_calls_string[0] = '\0';

        for (i = 0; i < expected_call_count; i++)
        {
            char* stringified_call = umockcall_stringify(expected_calls[i]);
            if (stringified_call == NULL)
            {
                break;
            }
            else
            {
                size_t stringified_call_length = strlen(stringified_call);
                new_expected_calls_string = (char*)realloc(expected_calls_string, current_length + stringified_call_length + 1);
                if (new_expected_calls_string == NULL)
                {
                    break;
                }
                else
                {
                    expected_calls_string = new_expected_calls_string;
                    (void)memcpy(expected_calls_string + current_length, stringified_call, stringified_call_length + 1);
                    current_length += stringified_call_length;
                }

                free(stringified_call);
            }
        }

        if (i < expected_call_count)
        {
            result = NULL;
        }
        else
        {
            result = expected_calls_string;
        }
    }

    return result;
}

const char* umock_c_get_actual_calls(void)
{
    const char* result;
    size_t i;
    char* new_actual_calls_string;

    new_actual_calls_string = (char*)realloc(actual_calls_string, 1);
    if (new_actual_calls_string == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t current_length = 0;
        actual_calls_string = new_actual_calls_string;
        actual_calls_string[0] = '\0';

        for (i = 0; i < actual_call_count; i++)
        {
            char* stringified_call = umockcall_stringify(actual_calls[i]);
            if (stringified_call == NULL)
            {
                break;
            }
            else
            {
                size_t stringified_call_length = strlen(stringified_call);
                new_actual_calls_string = (char*)realloc(actual_calls_string, current_length + stringified_call_length + 1);
                if (new_actual_calls_string == NULL)
                {
                    break;
                }
                else
                {
                    actual_calls_string = new_actual_calls_string;
                    (void)memcpy(actual_calls_string + current_length, stringified_call, stringified_call_length + 1);
                    current_length += stringified_call_length;
                }

                free(stringified_call);
            }
        }

        if (i < actual_call_count)
        {
            result = NULL;
        }
        else
        {
            result = actual_calls_string;
        }
    }

    return result;
}

UMOCKCALL_HANDLE umock_c_get_last_expected_call(void)
{
    UMOCKCALL_HANDLE result;

    if (expected_call_count == 0)
    {
        result = NULL;
    }
    else
    {
        result = expected_calls[expected_call_count - 1];
    }

    return result;
}
