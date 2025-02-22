/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_videolan_Libbluray */

#ifndef _Included_org_videolan_Libbluray
#define _Included_org_videolan_Libbluray
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_videolan_Libbluray
 * Method:    getTitleInfoN
 * Signature: (JI)Lorg/videolan/TitleInfo;
 */
JNIEXPORT jobject JNICALL Java_org_videolan_Libbluray_getTitleInfoN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getPlaylistInfoN
 * Signature: (JI)Lorg/videolan/TitleInfo;
 */
JNIEXPORT jobject JNICALL Java_org_videolan_Libbluray_getPlaylistInfoN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getTitlesN
 * Signature: (JC)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_getTitlesN
  (JNIEnv *, jclass, jlong, jchar);

/*
 * Class:     org_videolan_Libbluray
 * Method:    seekN
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_seekN
  (JNIEnv *, jclass, jlong, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    seekTimeN
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_seekTimeN
  (JNIEnv *, jclass, jlong, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    seekChapterN
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_seekChapterN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    chapterPosN
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_chapterPosN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getCurrentChapterN
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_getCurrentChapterN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    seekMarkN
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_seekMarkN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    selectPlaylistN
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_selectPlaylistN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    selectTitleN
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_selectTitleN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    selectAngleN
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_selectAngleN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    seamlessAngleChangeN
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_org_videolan_Libbluray_seamlessAngleChangeN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getTitleSizeN
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_getTitleSizeN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getCurrentTitleN
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_getCurrentTitleN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    getCurrentAngleN
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_getCurrentAngleN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    tellN
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_tellN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    tellTimeN
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_videolan_Libbluray_tellTimeN
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_videolan_Libbluray
 * Method:    writeGPRN
 * Signature: (JII)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_writeGPRN
  (JNIEnv *, jclass, jlong, jint, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    readGPRN
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_readGPRN
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     org_videolan_Libbluray
 * Method:    readPSRN
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_org_videolan_Libbluray_readPSRN
  (JNIEnv *, jclass, jlong, jint);

#ifdef __cplusplus
}
#endif
#endif
