#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
int main()
{
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec4 PO = projection * view * glm::vec4(0, 0, 0, 1.0f);
    float pl=PO.x/PO.w;
    glm::vec4 PO1 = projection * view * glm::vec4(1000, 0, 0, 1.0f);
    float pl1=PO1.x/PO1.w;

}
