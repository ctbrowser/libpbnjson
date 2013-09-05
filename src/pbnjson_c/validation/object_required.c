// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include "object_required.h"
#include "uri_resolver.h"
#include "validator.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void _release(Feature *f)
{
	ObjectRequired *o = (ObjectRequired *) f;
	g_hash_table_destroy(o->keys);
	g_free(o);
}

static bool _apply(Feature *f, Validator *v)
{
	ObjectRequired *o = (ObjectRequired *) f;
	validator_set_object_required(v, o);
	return true;
}

static FeatureVtable object_required_vtable =
{
	.release = _release,
	.apply = _apply,
};

ObjectRequired* object_required_new(void)
{
	ObjectRequired *o = g_new0(ObjectRequired, 1);
	if (!o)
		return NULL;

	o->keys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	if (!o->keys)
	{
		g_free(o);
		return NULL;
	}

	feature_init(&o->base, &object_required_vtable);
	return o;
}

ObjectRequired* object_required_ref(ObjectRequired *o)
{
	return (ObjectRequired *) feature_ref(&o->base);
}

void object_required_unref(ObjectRequired *o)
{
	feature_unref(&o->base);
}

guint object_required_size(ObjectRequired *o)
{
	return g_hash_table_size(o->keys);
}

bool object_required_add_key(ObjectRequired *o, char const *key)
{
	char *skey = g_strdup(key);
	if (!skey)
		return false;
	if (g_hash_table_lookup(o->keys, skey))
	{
		g_free(skey);
		return false;
	}
	g_hash_table_insert(o->keys, skey, skey);
	return true;
}

bool object_required_add_key_n(ObjectRequired *o, char const *key, size_t key_len)
{
	char *skey = g_strndup(key, key_len);
	if (!skey)
		return false;
	if (g_hash_table_lookup(o->keys, skey))
	{
		g_free(skey);
		return false;
	}
	g_hash_table_insert(o->keys, skey, skey);
	return o->keys;
}

char const *object_required_lookup_key(ObjectRequired *o, char const *key)
{
	return (char const *) g_hash_table_lookup(o->keys, key);
}

char const *object_required_lookup_key_n(ObjectRequired *o, char const *key, size_t key_len)
{
	char buffer[key_len + 1];
	strncpy(buffer, key, key_len);
	buffer[key_len] = 0;
	return object_required_lookup_key(o, buffer);
}
