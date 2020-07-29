#include "gtest/gtest.h"

#include <copentimelineio/clip.h>
#include <copentimelineio/deserialization.h>
#include <copentimelineio/errorStatus.h>
#include <copentimelineio/gap.h>
#include <copentimelineio/missingReference.h>
#include <copentimelineio/safely_typed_any.h>
#include <copentimelineio/serializableCollection.h>
#include <copentimelineio/serializableObject.h>
#include <copentimelineio/serializableObjectWithMetadata.h>
#include <copentimelineio/serialization.h>
#include <copentimelineio/timeline.h>
#include <copentimelineio/track.h>
#include <copentimelineio/transition.h>

class OpenTimeTypeSerializerTests : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

class SerializableObjectTests : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(OpenTimeTypeSerializerTests, SerializeTest)
{
    RationalTime* rt = RationalTime_create(15, 24);

    Any*             rt_any      = create_safely_typed_any_rational_time(rt);
    OTIOErrorStatus* errorStatus = OTIOErrorStatus_create();
    const char*      encoded = serialize_json_to_string(rt_any, errorStatus, 4);
    Any*             decoded = /* allocate memory for destinantion */
        create_safely_typed_any_rational_time(rt);

    bool decoded_successfully =
        deserialize_json_from_string(encoded, decoded, errorStatus);
    ASSERT_TRUE(decoded_successfully);
    RationalTime* decoded_rt = safely_cast_rational_time_any(decoded);

    EXPECT_TRUE(RationalTime_equal(rt, decoded_rt));
    Any_destroy(decoded);
    decoded = NULL;

    RationalTime* rt_dur = RationalTime_create(10, 20);
    TimeRange* tr = TimeRange_create_with_start_time_and_duration(rt, rt_dur);
    Any*       tr_any = create_safely_typed_any_time_range(tr);
    encoded           = serialize_json_to_string(tr_any, errorStatus, 4);
    decoded           = /* allocate memory for destinantion */
        create_safely_typed_any_time_range(tr);

    decoded_successfully =
        deserialize_json_from_string(encoded, decoded, errorStatus);
    ASSERT_TRUE(decoded_successfully);
    TimeRange* decoded_tr = safely_cast_time_range_any(decoded);

    EXPECT_TRUE(TimeRange_equal(tr, decoded_tr));
    Any_destroy(decoded);
    decoded = NULL;

    TimeTransform* tt =
        TimeTransform_create_with_offset_scale_rate(rt, 1.5, 24);
    Any* tt_any = create_safely_typed_any_time_transform(tt);
    encoded     = serialize_json_to_string(tt_any, errorStatus, 4);
    decoded     = /* allocate memory for destinantion */
        create_safely_typed_any_time_transform(tt);

    decoded_successfully =
        deserialize_json_from_string(encoded, decoded, errorStatus);
    ASSERT_TRUE(decoded_successfully);
    TimeTransform* decoded_tt = safely_cast_time_transform_any(decoded);

    EXPECT_TRUE(TimeTransform_equal(tt, decoded_tt));
    Any_destroy(decoded);
    decoded = NULL;

    OTIOErrorStatus_destroy(errorStatus);
    errorStatus = NULL;
    RationalTime_destroy(rt);
    rt = NULL;
    RationalTime_destroy(rt_dur);
    rt_dur = NULL;
    RationalTime_destroy(decoded_rt);
    decoded_rt = NULL;
    TimeRange_destroy(tr);
    tr = NULL;
    TimeRange_destroy(decoded_tr);
    decoded_tr = NULL;
    TimeTransform_destroy(tt);
    tt = NULL;
    TimeTransform_destroy(decoded_tt);
    decoded_tt = NULL;
}

TEST_F(SerializableObjectTests, ConstructorTest)
{
    AnyDictionary*         metadata = AnyDictionary_create();
    Any*                   barAny   = create_safely_typed_any_string("bar");
    AnyDictionaryIterator* it = AnyDictionary_insert(metadata, "foo", barAny);
    AnyDictionaryIterator_destroy(it);
    it = NULL;

    SerializableObjectWithMetadata* so = SerializableObjectWithMetadata_create(NULL, metadata);
    RetainerSerializableObject* so_r = RetainerSerializableObject_create(reinterpret_cast<OTIOSerializableObject*>(so));

    AnyDictionary* metadataResult = SerializableObjectWithMetadata_metadata(so);
    it                            = AnyDictionary_find(metadataResult, "foo");
    Any*        fooValAny         = AnyDictionaryIterator_value(it);
    const char* fooValStr         = safely_cast_string_any(fooValAny);
    EXPECT_STREQ(fooValStr, "bar");

    AnyDictionaryIterator_destroy(it);
    it = NULL;
    Any_destroy(barAny);
    barAny = NULL;
    AnyDictionary_destroy(metadataResult);
    metadataResult = NULL;
    Any_destroy(fooValAny);
    fooValAny = NULL;
    RetainerSerializableObject_managed_destroy(so_r);
    so = NULL;
}

TEST_F(SerializableObjectTests, EqualityTest)
{
    OTIOSerializableObject* o1 = SerializableObject_create();
    OTIOSerializableObject* o2 = SerializableObject_create();
    RetainerSerializableObject* o1_r = RetainerSerializableObject_create(o1);
    RetainerSerializableObject* o2_r = RetainerSerializableObject_create(o2);
    EXPECT_NE(o1, o2);
    EXPECT_TRUE(SerializableObject_is_equivalent_to(o1, o2));
    RetainerSerializableObject_managed_destroy(o1_r);
    RetainerSerializableObject_managed_destroy(o2_r);
}

TEST_F(SerializableObjectTests, EquivalenceSymmetryTest)
{
    Composable* A = Composable_create();
    Composable* B = Composable_create();
    OTIOSerializableObject* A_o = (OTIOSerializableObject*) A;
    OTIOSerializableObject* B_o = (OTIOSerializableObject*) B;
    RetainerSerializableObject* A_r = RetainerSerializableObject_create(A_o);
    RetainerSerializableObject* B_r = RetainerSerializableObject_create(B_o);
    EXPECT_TRUE(Composable_is_equivalent_to(A, B_o));
    EXPECT_TRUE(Composable_is_equivalent_to(B, A_o));
    RetainerSerializableObject_managed_destroy(A_r);
    RetainerSerializableObject_managed_destroy(B_r);
}
