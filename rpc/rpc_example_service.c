#include "rpc_example_service.h"
#include <jansson.h>    // FIXME service should not depend on jansson directly

void
Person_free(Person* person)
{
	cx_free(person->firstname);
	cx_free(person->lastname);
	cx_free(person);
}

json_t*
Person_to_json(Person* person)
{
	json_t* json = json_object();

	json_object_set_new(json, "firstname", json_string(person->firstname));
	json_object_set_new(json, "lastname", json_string(person->lastname));
	// FIXME requires same integer deserialization as simple params
	json_object_set_new(json, "age", json_integer(person->age));
	return json;
}

/*
 * @return -1 on failure, 0 on success
 */
int
Person_from_json(Person* person, json_t* json)
{
	json_error_t json_error;
	int status = json_unpack_ex(json, &json_error, JSON_STRICT, "{s:s, s:s, s:i}",
				    "firstname", &person->firstname,
				    "lastname", &person->lastname,
				    "age", &person->age);


	if (status == -1)
	{
		cx_ferr_set(CX_RPC_ERROR_INVALID_PARAMS, "Failed to deserialize parameter 'person' : %s", json_error.text);
	}

	return status;
}

/* API methods */

static Person*
get_person()
{
	Person* person = cx_alloc(sizeof(Person));

	person->firstname = cx_strdup("Max");
	person->lastname = cx_strdup("Mustermann");
	person->age = 33;
	person->f_free = &Person_free;
	return person;
}

static char*
print_person(Person* person)
{
	char buf[1024];

	int num_printed = snprintf(buf, sizeof(buf), "%s %s (age %d)", person->firstname, person->lastname, person->age);

	if (num_printed >= (int)sizeof(buf))
	{
		// FIXME set error BUFFER_TO_SMALL
	}

	return cx_strdup(buf);
}

static bool
has_count(const char* string, size_t count)
{
	bool result = false;

	if (string)
	{
		printf("string:%s count:%zu\n", string, count);
		result = strlen(string) == count;
	}
	else
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'string' is null");
	}

	return result;
}

/* service wrapper */

static int
call_has_count(RPC_Param* params, int num_params, RPC_Value* result, RPC_Format format)
{
	/* typesafe parameter deserialization with error checking */
	RPC_Value* p_string = Param_get(params, 0, "string", num_params);

	if (p_string == NULL)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'string' unavailable");
		return -1;
	}

	if (p_string->type != RPC_TYPE_STRING)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'string' has invalid type (expected string)");
		return -1;
	}

	RPC_Value* p_count = Param_get(params, 1, "count", num_params);

	if (p_count == NULL)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'count' unavailable");
		return -1;
	}

	if (p_count->type != RPC_TYPE_INTEGER)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'count' has invalid type (expected integer)");
		return -1;
	}

	switch (format)
	{
	case FORMAT_NATIVE:
		break;
	case FORMAT_JSON:
		cx_err_set(CX_RPC_ERROR_INTERNAL, "Output format 'JSON' is not suppported by this method.");
		return -1;
	}

	/* call method */
	bool out = has_count(p_string->value.string, (size_t)p_count->value.integer);

	/* prepare result */
	result->type = RPC_TYPE_BOOLEAN;
	result->value.boolean = out;

	return 1;
}

static int
call_get_person(RPC_Param* params, int num_params, RPC_Value* result, RPC_Format format)
{
	/* method has no params */
	UNUSED(params);
	UNUSED(num_params);
	UNUSED(format);

	Person* person = get_person();

	result->type = RPC_TYPE_OBJECT;
	result->value.object = person;
	result->f_to_json = (F_ValueToJSON*)&Person_to_json;
	result->f_free = (F_RPC_ValueFree*)&Person_free;

	return 1;
}

static void
simple_free(void* object)
{
	cx_free(object);
}

static int
call_print_person(RPC_Param* params, int num_params, RPC_Value* result, RPC_Format format)
{
	UNUSED(result);

	/* typesafe parameter deserialization with error checking */
	RPC_Value* p_person = Param_get(params, 0, "person", num_params);

	if (p_person == NULL)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'person' missing");
		return -1;
	}

	if (p_person->type != RPC_TYPE_OBJECT)
	{
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "Parameter 'person' has invalid type (expected object)");
		return -1;
	}

	/* conversion of parameters in non-native format e.g json */
	char* person_s = NULL;
	switch (format)
	{
	case FORMAT_NATIVE:
	{
		Person* person = (Person*)p_person->value.object;
		person_s = print_person(person);
		break;
	}
	case FORMAT_JSON:
	{
		Person person;
		memset(&person, 0, sizeof(Person));
		if (Person_from_json(&person, (json_t*)p_person->value.object) == 0)
			person_s = print_person(&person);
		else
		{
			return -1;
		}
		break;
	}
	}

	result->type = RPC_TYPE_STRING;
	result->value.string = person_s;
	result->f_free = &simple_free;

	return 0;
}

RPC_MethodTable EXAMPLE_SERVICE_METHODS[] =
{
	{ "has_count", call_has_count },
	{ "get_person", call_get_person },
	{ "print_person", call_print_person },
	{ NULL, NULL }
};
