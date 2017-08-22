#include "gl/renderer.h"

std::string RTTFragmentShader() {
  return "uniform sampler2D color_texture;\n"
         "varying vec2 texture_coordinate;\n"
         "void main() {\n"
         "  gl_FragColor.rgb = texture2D(color_texture, texture_coordinate).rgb;\n"
         "}\n";
}

std::string RTTVertexShader() {
  return "varying vec2 texture_coordinate;\n"
         "attribute vec3 v_vertex;\n"
         "attribute vec2 v_coord;\n"
         "void main()\n"
         "{\n"
         "    texture_coordinate = v_coord.st;\n"
         "    gl_Position = vec4(v_vertex.xy, 0.0, 1.0);\n"
         "}\n";
}

namespace oc {
    GLRenderer::GLRenderer() {
        fboID = 0;
        rboID = 0;
        rendertexture = 0;
        scene = 0;
    }

    GLRenderer::~GLRenderer() {
        Cleanup();
    }

    void GLRenderer::Cleanup() {
        if (fboID) {
            glDeleteFramebuffers(1, fboID);
            delete[] fboID;
            fboID = 0;
        }
        if (rboID) {
            glDeleteRenderbuffers(1, rboID);
            delete[] rboID;
            rboID = 0;
        }
        if (rendertexture) {
            glDeleteTextures(1, rendertexture);
            delete[] rendertexture;
            rendertexture = 0;
        }
        if (scene) {
            delete scene;
            scene = 0;
        }
    }

    void GLRenderer::Init(int w, int h, float a) {
        aliasing = a;
        width = w;
        height = h;
        Cleanup();

        //find ideal texture resolution
        int resolution = 2;
        while (resolution < width)
            resolution *= 2;

        //create frame buffer
        fboID = new unsigned int[1];
        rboID = new unsigned int[1];
        rendertexture = new unsigned int[1];

        //framebuffer textures
        glGenTextures(1, rendertexture);
        glBindTexture(GL_TEXTURE_2D, rendertexture[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        /// create render buffer for depth buffer
        glGenRenderbuffers(1, rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, rboID[0]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

        //framebuffers
        glGenFramebuffers(1, fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendertexture[0], 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[0]);

        /// check FBO status
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::exit(EXIT_SUCCESS);

        //set viewport
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport (0, 0, (GLsizei) width, (GLsizei) height);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture( GL_TEXTURE0 );

        //set objects
        camera.projection = glm::perspective(glm::radians(45.0f), w / (float)h, 0.1f, 1000.0f);
        scene = new GLSL(RTTVertexShader(), RTTFragmentShader());
    }

    void GLRenderer::Render(float* vertices, float* normals, float* uv, unsigned int* colors,
                            unsigned long size, unsigned int* indices) {
        GLSL::CurrentShader()->UniformMatrix("MVP", glm::value_ptr(camera.projection * camera.GetView()));
        GLSL::CurrentShader()->Attrib(vertices, normals, uv, colors);

        if (size > 0) {
            if (indices)
              glDrawElements(GL_TRIANGLES, (GLsizei) size, GL_UNSIGNED_INT, indices);
            else
              glDrawArrays(GL_TRIANGLES, 0, (GLsizei) size);
        }
    }

    void GLRenderer::Rtt(bool enable) {
        if (enable) {
            glBindFramebuffer(GL_FRAMEBUFFER, fboID[0]);
            glViewport (0, 0, (GLsizei) (width * aliasing), (GLsizei) (height * aliasing));
            glClearStencil(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        } else {
            /// prepare rendering
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport (0, 0, width, height);

            /// vertices
            glm::vec3 vertices[] = {
                glm::vec3(-1, +1, 0),
                glm::vec3(-1, -1, 0),
                glm::vec3(+1, -1, 0),
                glm::vec3(-1, +1, 0),
                glm::vec3(+1, -1, 0),
                glm::vec3(+1, +1, 0),
            };

            /// coords
            glm::vec2 coords[] = {
                glm::vec2(0, aliasing),
                glm::vec2(0, 0),
                glm::vec2(aliasing, 0),
                glm::vec2(0, aliasing),
                glm::vec2(aliasing, 0),
                glm::vec2(aliasing, aliasing),
            };

            scene->Bind();
            glBindTexture(GL_TEXTURE_2D, rendertexture[0]);
            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            /// render
            scene->Attrib(&vertices[0].x, 0, &coords[0].s, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDepthMask(GL_TRUE);
        }
    }
}
