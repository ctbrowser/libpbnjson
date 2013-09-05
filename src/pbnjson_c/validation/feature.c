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

#include "feature.h"

void feature_init(Feature *f, FeatureVtable *vtable)
{
	f->ref_count = 1;
	f->vtable = vtable;
}

Feature* feature_ref(Feature *f)
{
	if (!f)
		return f;
	++f->ref_count;
	return f;
}

void feature_unref(Feature *f)
{
	if (!f || --f->ref_count)
		return;
	if (!f->vtable || !f->vtable->release)
		return;
	f->vtable->release(f);
}

bool feature_apply(Feature *f, Validator *v)
{
	if (!f || !f->vtable || !f->vtable->apply)
		return false;
	return f->vtable->apply(f, v);
}
