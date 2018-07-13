#include "editor/rasterizer.h"

namespace oc {

    void Rasterizer::AddUVS(std::vector<glm::vec2> uvs, std::vector<unsigned int> selected) {
        glm::vec3 a, b, c;
        for (unsigned long i = 0; i < uvs.size(); i += 3) {
            if (!selected.empty()) {
                if (selected[i + 0] != 0)
                    continue;
                if (selected[i + 1] != 0)
                    continue;
                if (selected[i + 2] != 0)
                    continue;
            }
            //get coordinate
            a = glm::vec3(uvs[i + 0], 0.0f);
            b = glm::vec3(uvs[i + 1], 0.0f);
            c = glm::vec3(uvs[i + 2], 0.0f);
            //mirror y axis
            a.y = 1.0f - a.y;
            b.y = 1.0f - b.y;
            c.y = 1.0f - c.y;
            //scale into raster dimensions
            a.x *= (float)(viewport_width - 1);
            a.y *= (float)(viewport_height - 1);
            b.x *= (float)(viewport_width - 1);
            b.y *= (float)(viewport_height - 1);
            c.x *= (float)(viewport_width - 1);
            c.y *= (float)(viewport_height - 1);
            //process
            Triangle(i, a, b, c);
        }
    }

    void Rasterizer::AddVertices(std::vector<glm::vec3>& vertices, glm::mat4 world2screen, bool culling) {
        glm::vec3 a, b, c, ba, ca;
        glm::vec4 wa, wb, wc;
        for (unsigned long i = 0; i < vertices.size(); i += 3) {
            //get coordinate
            wa = glm::vec4(vertices[i + 0], 1.0f);
            wb = glm::vec4(vertices[i + 1], 1.0f);
            wc = glm::vec4(vertices[i + 2], 1.0f);
            //transform to 2D
            wa = world2screen * wa;
            wb = world2screen * wb;
            wc = world2screen * wc;
            //perspective division
            wa /= glm::abs(wa.w);
            wb /= glm::abs(wb.w);
            wc /= glm::abs(wc.w);
            //convert
            a.x = wa.x;
            a.y = wa.y;
            a.z = wa.z;
            b.x = wb.x;
            b.y = wb.y;
            b.z = wb.z;
            c.x = wc.x;
            c.y = wc.y;
            c.z = wc.z;
            //convert it from -1,1 to 0,1
            a.x = (a.x + 1.0f) * 0.5f;
            a.y = (a.y + 1.0f) * 0.5f;
            b.x = (b.x + 1.0f) * 0.5f;
            b.y = (b.y + 1.0f) * 0.5f;
            c.x = (c.x + 1.0f) * 0.5f;
            c.y = (c.y + 1.0f) * 0.5f;
            //scale into raster dimensions
            a.x *= (float)(viewport_width - 1);
            a.y *= (float)(viewport_height - 1);
            b.x *= (float)(viewport_width - 1);
            b.y *= (float)(viewport_height - 1);
            c.x *= (float)(viewport_width - 1);
            c.y *= (float)(viewport_height - 1);
            //back face culling
            if (culling) {
                ba = glm::vec3(b.x - a.x, b.y - a.y, 0.0f);
                ca = glm::vec3(c.x - a.x, c.y - a.y, 0.0f);
                if (glm::cross(ba, ca).z < 0)
                    continue;
            }
            //process
            Triangle(i, a, b, c);
        }
    }

    void Rasterizer::AddUVVertices(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, glm::mat4 world2screen,
                                   float cx, float cy, float fx, float fy) {
        glm::vec3 a, b, c;
        glm::vec4 wa, wb, wc;
        for (unsigned long i = 0; i < vertices.size(); i += 3) {
            //get coordinate
            wa = glm::vec4(vertices[i + 0], 1.0f);
            wb = glm::vec4(vertices[i + 1], 1.0f);
            wc = glm::vec4(vertices[i + 2], 1.0f);
            //transform to 2D
            wa = world2screen * wa;
            wb = world2screen * wb;
            wc = world2screen * wc;
            //perspective division
            wa /= glm::abs(wa.z * wa.w);
            wb /= glm::abs(wb.z * wb.w);
            wc /= glm::abs(wc.z * wc.w);
            //apply calibration
            wa.x *= fx; wa.y *= fy; wa.x += cx; wa.y += cy;
            wb.x *= fx; wb.y *= fy; wb.x += cx; wb.y += cy;
            wc.x *= fx; wc.y *= fy; wc.x += cx; wc.y += cy;
            //convert
            a.x = wa.x;
            a.y = wa.y;
            a.z = wa.z;
            b.x = wb.x;
            b.y = wb.y;
            b.z = wb.z;
            c.x = wc.x;
            c.y = wc.y;
            c.z = wc.z;
            //convert it from -1,1 to 0,1
            a.x = (a.x + 1.0f) * 0.5f;
            a.y = (a.y + 1.0f) * 0.5f;
            b.x = (b.x + 1.0f) * 0.5f;
            b.y = (b.y + 1.0f) * 0.5f;
            c.x = (c.x + 1.0f) * 0.5f;
            c.y = (c.y + 1.0f) * 0.5f;
            //scale into raster dimensions
            a.x *= (float)(viewport_width - 1);
            a.y *= (float)(viewport_height - 1);
            b.x *= (float)(viewport_width - 1);
            b.y *= (float)(viewport_height - 1);
            c.x *= (float)(viewport_width - 1);
            c.y *= (float)(viewport_height - 1);
            //process
            Triangle(i, a, b, c, uvs[i + 0], uvs[i + 1], uvs[i + 2]);
        }
    }

    void Rasterizer::SetResolution(int w, int h) {
        viewport_width = w;
        viewport_height = h;

        if (fillCache1.size() != h + 1)
            fillCache1.resize((unsigned long) (h + 1));
        if (fillCache2.size() != h + 1)
            fillCache2.resize((unsigned long) (h + 1));
    }

    bool Rasterizer::Line(int x1, int y1, int x2, int y2, glm::dvec3 z1, glm::dvec3 z2,
                          std::pair<int, glm::dvec3>* fillCache) {

        //Liang & Barsky clipping (only top-bottom)
        int h = y2 - y1;
        double t1 = 0, t2 = 1;
        if (Test(-h, y1, t1, t2) && Test(h, viewport_height - 1 - y1, t1, t2) ) {
            glm::dvec3 z;
            int c0, c1, xp0, xp1, yp0, yp1, y, p, w;
            bool wp, hp;

            //clip line
            if (t1 > 0) {
                w = x2 - x1;
                z = z2 - z1;
                x1 += t1 * w;
                y1 += t1 * h;
                z1 += t1 * z;
            } else
                t1 = 0;
            if (t2 < 1) {
                w = x2 - x1;
                z = z2 - z1;
                t2 -= t1;
                x2 = (int) (x1 + t2 * w);
                y2 = (int) (y1 + t2 * h);
                z2 = z1 + t2 * z;
            }

            //count new line dimensions
            wp = x2 >= x1;
            w = wp ? x2 - x1 : x1 - x2;
            hp = y2 >= y1;
            h = hp ? y2 - y1 : y1 - y2;

            //line in x axis nearby
            if (w > h) {
                //direction from left to right
                xp0 = wp ? 1 : -1;
                yp0 = 0;

                //direction from top to bottom
                xp1 = wp ? 1 : -1;
                yp1 = hp ? 1 : -1;

                //line in y axis nearby
            } else {
                //direction from top to bottom
                xp0 = 0;
                yp0 = hp ? 1 : -1;

                //direction from left to right
                xp1 = wp ? 1 : -1;
                yp1 = hp ? 1 : -1;

                //apply line length
                y = w;
                w = h;
                h = y;
            }

            //count z coordinate step
            z = (z2 - z1) / (double)w;

            //Bresenham's algorithm
            c0 = h + h;
            p = c0 - w;
            c1 = p - w;
            y = y1;
            fillCache[y].first = x1;
            fillCache[y].second = z1;
            for (w--; w >= 0; w--) {

                //interpolate
                if (p < 0) {
                    p += c0;
                    x1 += xp0;
                    y1 += yp0;
                } else {
                    p += c1;
                    x1 += xp1;
                    y1 += yp1;
                }
                z1 += z;

                //write cache info
                if (wp || (y != y1)) {
                    y = y1;
                    fillCache[y].first = x1;
                    fillCache[y].second = z1;
                }
            }
            return true;
        }
        return false;
    }

    bool Rasterizer::Test(double p, double q, double &t1, double &t2) {
        //negative cutting
        if (p < 0) {
            double t = q/p;

            //cut nothing
            if (t > t2)
                return false;
                //cut the first coordinate
            else if (t > t1)
                t1 = t;

            //positive cutting
        } else if (p > 0) {
            double t = q/p;

            //cut nothing
            if (t < t1)
                return false;
                //cut the second coordinate
            else if (t < t2)
                t2 = t;

            //line is right to left(or bottom to top)
        } else if (q < 0)
            return false;
        return true;
    }

    void Rasterizer::Triangle(unsigned long& index, glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec2 ta, glm::vec2 tb, glm::vec2 tc) {

        //create markers for filling
        int min, max;
        int ab = (int) glm::abs(a.y - b.y);
        int ac = (int) glm::abs(a.y - c.y);
        int bc = (int) glm::abs(b.y - c.y);
        glm::ivec2 ia = glm::ivec2(a.x + 0.5f, a.y + 0.5f);
        glm::ivec2 ib = glm::ivec2(b.x + 0.5f, b.y + 0.5f);
        glm::ivec2 ic = glm::ivec2(c.x + 0.5f, c.y + 0.5f);
        if ((ab >= ac) && (ab >= bc)) {
            Line(ia.x, ia.y, ib.x, ib.y, glm::dvec3(ta, a.z), glm::dvec3(tb, b.z), &fillCache1[0]);
            Line(ia.x, ia.y, ic.x, ic.y, glm::dvec3(ta, a.z), glm::dvec3(tc, c.z), &fillCache2[0]);
            Line(ib.x, ib.y, ic.x, ic.y, glm::dvec3(tb, b.z), glm::dvec3(tc, c.z), &fillCache2[0]);
            min = glm::max(0, glm::min(ia.y, ib.y));
            max = glm::min(glm::max(ia.y, ib.y), viewport_height - 1);
        } else if ((ac >= ab) && (ac >= bc)) {
            Line(ia.x, ia.y, ic.x, ic.y, glm::dvec3(ta, a.z), glm::dvec3(tc, c.z), &fillCache1[0]);
            Line(ia.x, ia.y, ib.x, ib.y, glm::dvec3(ta, a.z), glm::dvec3(tb, b.z), &fillCache2[0]);
            Line(ib.x, ib.y, ic.x, ic.y, glm::dvec3(tb, b.z), glm::dvec3(tc, c.z), &fillCache2[0]);
            min = glm::max(0, glm::min(ia.y, ic.y));
            max = glm::min(glm::max(ia.y, ic.y), viewport_height - 1);
        } else {
            Line(ib.x, ib.y, ic.x, ic.y, glm::dvec3(tb, b.z), glm::dvec3(tc, c.z), &fillCache1[0]);
            Line(ia.x, ia.y, ib.x, ib.y, glm::dvec3(ta, a.z), glm::dvec3(tb, b.z), &fillCache2[0]);
            Line(ia.x, ia.y, ic.x, ic.y, glm::dvec3(ta, a.z), glm::dvec3(tc, c.z), &fillCache2[0]);
            min = glm::max(0, glm::min(ib.y, ic.y));
            max = glm::min(glm::max(ib.y, ic.y), viewport_height - 1);
        }

        //fill triangle
        int memy = min * viewport_width;
        for (int y = min; y <= max; y++) {
            int x1 = fillCache1[y].first;
            int x2 = fillCache2[y].first;
            glm::dvec3 z1 = fillCache1[y].second;
            glm::dvec3 z2 = fillCache2[y].second;

            //Liang & Barsky clipping
            double t1 = 0;
            double t2 = 1;
            int x = x2 - x1;
            if (Test(-x, x1, t1, t2) && Test(x, viewport_width - 1 - x1, t1, t2)) {

                //callback for processing
                if (x2 > x1)
                    Process(index, x1, x2, y, z1, z2);
                else
                    Process(index, x2, x1, y, z2, z1);
            }
            memy += viewport_width;
        }
    }
}
