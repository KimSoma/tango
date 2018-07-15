#ifndef EDITOR_SELECTOR_H
#define EDITOR_SELECTOR_H

#include "data/mesh.h"
#include "editor/rasterizer.h"

#define DESELECT_COLOR 0x00204040

namespace oc {

class Selector : Rasterizer {
public:
    void CompleteSelection(std::vector<Mesh>& mesh, bool inverse);
    void DecreaseSelection(std::vector<Mesh>& mesh);
    glm::vec3 GetCenter(std::vector<Mesh>& mesh);
    void IncreaseSelection(std::vector<Mesh>& mesh);
    void Init(int w, int h);
    void SelectObject(std::vector<Mesh>& mesh, glm::mat4 world2screen, float x, float y);
    void SelectRect(std::vector<Mesh>& mesh, glm::mat4 world2screen, float x1, float y1, float x2, float y2);
    void SelectTriangle(std::vector<Mesh>& mesh, glm::mat4 world2screen, float x, float y);

private:
    virtual void Process(unsigned long& index, int &x1, int &x2, int &y, glm::dvec3 &z1, glm::dvec3 &z2);
    std::string VertexToKey(glm::vec3& vec);

    std::map<std::string, std::map<std::pair<int, int>, bool> > connections;
    Mesh* currentMesh;
    double depth;
    int pointX, pointY, pointX2, pointY2;
    bool rangeMode;
    int selected;
};
}

#endif
