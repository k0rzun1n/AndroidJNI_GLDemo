/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.krz.DemoGLJNIYo;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class GLES3JNIView extends GLSurfaceView {
    private static final String TAG = "GLES3JNI";
    private static final boolean DEBUG = true;

    public GLES3JNIView(Context context) {
        super(context);
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        Renderer r = new Renderer();
        r.setAssetManager(context.getAssets());
        setRenderer(r);
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        private AssetManager mAssetManager;

        public void setAssetManager(AssetManager am) {
            mAssetManager = am;
        }


        public void onDrawFrame(GL10 gl) {
            GLES3JNILib.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Log.d("","Activity onSurfaceChanged");
            GLES3JNILib.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            //gets called on every screen turn
            Log.d("","Activity onSurfaceCreated");
            GLES3JNILib.init(mAssetManager);
        }
    }
}
