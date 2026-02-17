#include "project.h"

Project::Project()
{
    m_canvas = new Canvas();
}

Project::~Project()
{
    delete m_canvas;
}
