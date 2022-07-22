#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/gldraw.h>
#include <Eigen/Core>
#include <vector>
#include <string>
#include <pangolin/pangolin.h>
#include <unistd.h> 
#include <errno.h>
#include <pangolin/display/widgets.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>

#include<pangolin/gl/opengl_render_state.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>

using namespace std;
using namespace Eigen; 

#include<bits/stdc++.h>

#include <chrono>   //计算时间
using namespace std::chrono;






static void setnonblocking(int sockfd)
{
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) 
    {
        printf("fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) 
    {
        printf("fcntl F_SETFL fail");
    }
} 

bool drawPoint = false;
Eigen::Matrix4f m4;



const char *vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec4 ourColor;\n"
    "uniform float aAlpha;\n"
    "uniform int aPointSize;\n"
    "uniform float aW;\n"
    // "uniform mat4 aP;\n"
    // "uniform mat4 aMV;\n"
    "void main()\n"
    "{\n"
    "   gl_Position =  vec4 (aPos, aW);\n"
    "   ourColor = vec4(aColor, aAlpha);\n"
    "   gl_PointSize =  aPointSize;\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "in vec4 ourColor;\n"
    // "layout(location = 1) out vec4 gl_FragColor;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = ourColor;\n"
    "}\n\0";




int main(int argc , char *argv[]){


    pangolin::CreateWindowAndBind("Main",1920,1080);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const int UI_WIDTH = 20* pangolin::default_font().MaxWidth();
    
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Define Projection and initial ModelView matrix
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(192,108,420,420,320,240,0.2,10000),
        pangolin::ModelViewLookAt(1,1,-1, 20,0,0, pangolin::AxisY)
    );

    // Create Interactive View in window
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
            .SetBounds(0.0, 1.0, 0.0, 1.0, -1920.0f/1080.0f)
            .SetHandler(&handler);
    pangolin::CreatePanel("ui")
            .SetBounds(0.0, 1.0, 0.0,  pangolin::Attach::Pix(UI_WIDTH));
            

    pangolin::Var<int> point_Size("ui.Point_Size", 10, 1, 30);
    pangolin::Var<float> alpha("ui.Alpha", 1.0f, 0.0f, 1.0f);
    pangolin::Var<float> w_Size("ui.W_Size", 1.0f, -100.0f, 200.0f);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);


    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1 ,&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // std::string path = "./shader";
    // std::string sphere_vs = "main.vert";
    // std::string sphere_fs = "main.frag";
    // std::shared_ptr<FGLTools::Shader> shader = loadProgramFromFile(path, sphere_vs, sphere_fs);


    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
     cout << "Server:\n";
        if(sock_fd < 0) {
            perror("socket 创建失败");
            return 0;
        }
        setnonblocking(sock_fd);
        cout << "socket 创建成功!\n";
        // 绑定 ip port
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8000);
        // inet_pton(AF_INET, "192.168.0.111", &addr.sin_addr.s_addr);
        addr.sin_addr.s_addr = inet_addr("10.10.134.124"); //INADDR_ANY 通配地址，即本机所有 ip 都绑定上。 INADDR_ANY 转换过来就是0.0.0.0
        int res = bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
        if(res < 0) {
            perror("绑定失败");
            close(sock_fd);
            return 0;
        }
        cout << "socket 绑定（命名）成功！\n";
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);       
        vector<Vector4d, Eigen::aligned_allocator<Vector4d>> pointCloud;
        vector<Vector4d, Eigen::aligned_allocator<Vector4d>> pointColor;

        vector<float> pointFloat = {0, 0, 0};

        vector<float> colorFloat = {0, 0, 0};


        int pointCount = 1;

        int SizeofVertices = 1;

        // glUseProgram(shaderProgram);


        // float alpha = 0.1f;

    while( ! pangolin::ShouldQuit() )
    {


            // glClearColor (1.0, 1.0, 1.0, 1.0);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shaderProgram);

            //  glClearColor(0.64f, 0.5f, 0.81f, 0.0f);
            d_cam.Activate(s_cam);

            pangolin::OpenGlMatrix P = s_cam.GetProjectionMatrix();
            pangolin::OpenGlMatrix MV = s_cam.GetModelViewMatrix();


            int vertexSizeLocation = glGetUniformLocation(shaderProgram, "aPointSize");
		    glUniform1i(vertexSizeLocation, point_Size);
            int vertexAlphaLocation = glGetUniformLocation(shaderProgram, "aAlpha");
		    glUniform1f(vertexAlphaLocation, alpha);
            int vertexWLocation = glGetUniformLocation(shaderProgram, "aW");
		    glUniform1f(vertexWLocation, w_Size);
            // int vertexPLocation = glGetUniformLocation(shaderProgram, "aP");
            // glUniformMatrix4fv(vertexPLocation, 1, GL_FALSE, P.SetIdentity());
            // int vertexMVLocation = glGetUniformLocation(shaderProgram, "aMV");
            // glUniformMatrix4fv(vertexPLocation, 1, GL_FALSE, MV);
            // glPointSize(point_Size);
            // int vertexColorLocation = glGetUniformLocation(shaderProgram, "Alpha");
            // glUniform1f(vertexColorLocation,alpha);
            unsigned char rgbi[4];
            // glUseProgram(shaderProgram);
                // Render OpenGL Cube

                // pangolin::glDrawColouredCube();

                // Swap frames and Process Events


            float read_buf[12001] = {0.0};
            float test[12001] = {0.0};
            recvfrom(sock_fd, read_buf, sizeof(read_buf), 0, NULL, NULL);
            if(read_buf[0] != 0.0){
                int data_i = 0;
                for(int i = 0; i < 2250; i ++)
                {
                    // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
                    pointCount++;
                    Vector4d point (0, 0, 0, 1/255.0);
                    Vector4d color (1, 0, 1, 1);
                    point[0] = read_buf[4 * data_i + 0];
                    point[1] = read_buf[4 * data_i + 1];
                    point[2] = read_buf[4 * data_i + 2];
                    memcpy(rgbi, &(read_buf[4 * data_i + 3]), sizeof(float));
                    color[0] = (float)rgbi[0]/255;
                    color[1] = (float)rgbi[1]/255;
                    color[2] = (float)rgbi[2]/255;
                    color[3] = (float)alpha;
                    
                    printf("x: %lf, y: %lf, z: %lf, r: %d, g: %d, b: %d, i: %d. \n", point[0], \
                            point[1], point[2], rgbi[0], rgbi[1], rgbi[2],rgbi[3]);
                    pointCloud.push_back(point);
                    pointColor.push_back(color);

                    // pointFloat.push_back(point);
                    
                    pointFloat.push_back(read_buf[4 * data_i + 0]);
                    pointFloat.push_back(read_buf[4 * data_i + 1]);
                    pointFloat.push_back(read_buf[4 * data_i + 2]);

                    pointFloat.push_back(color[0]);
                    pointFloat.push_back(color[1]);
                    pointFloat.push_back(color[2]);

                    colorFloat.push_back(color[0]);
                    colorFloat.push_back(color[1]);
                    colorFloat.push_back(color[2]);
                    // colorFloat.push_back(alpha);

                    data_i ++;
                    // colorFloat.push_back(alpha);
                    // glUniform4f(vertexColorLocation,1,1,1, alpha);
                }
            }


            // int count = 0;
            // auto starttime = system_clock::now();

    //      
            // GLfloat vertices[pointFloat.size()];
            // memcpy(vertices, &pointFloat[0], pointFloat.size() * sizeof(pointFloat[0]));

            // for(int i = 0; i != sizeof(vertices);++i)
            // {
            //     cout << vertices[i] <<endl;
            // }
            // float vertices[1000000000000000];

            // for(int i = 0; i < pointFloat.size(); i++)
            // {
            //     vertices[i] = pointFloat[i];
            // }





            GLfloat* vertices = &pointFloat[3];
            // GLfloat* colors = &colorFloat[3];



            


            //旧VBO
            // float vertices[] = {
            //     // positions         // colors
            //     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
            //     -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
            //     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // top 
            //     0.25f,-0.25f,0.0f, 0.53f,0.13f,0.67f,
            //     0.0f,-0.5f,0.0f, 0.81f,0.47f,0.13f,
            //     -0.25f,-0.25f,0.0f,0.13f,0.13f,0.08f,
            //     -1.0f,1.0f,-0.0f,0.47f,0.47f,0.13f,
            //     0.13f,0.14f,0.0f,0.10f,0.87f,0.87f

            // };


            unsigned int vao,vbo;
            glGenVertexArrays(1, &vao); 
            glGenBuffers(1, &vbo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);

            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * pointCount, vertices, GL_STATIC_DRAW);
            // glBufferData(GL_ARRAY_BUFFER,  3 * sizeof(GLfloat) * pointCount, vertices, GL_STATIC_DRAW);
            // glUseProgram(shaderProgram);


            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(sizeof(float) * 3));
            glEnableVertexAttribArray(1);

            // glGenBuffers(1, &vbo);

            // glUseProgram(shaderProgram);
            // glBindVertexArray(vao);

            // glDrawArrays(GL_TRIANGLES, 0, 8);

            // glBindVertexArray(0);
            

            //新vbo
            // pangolin::GlBuffer glxyz(pangolin::GlArrayBuffer, pointCount , GL_FLOAT, 3, GL_STATIC_DRAW );
            // glxyz.Upload(vertices, 3 * sizeof(GLfloat) * pointCount);
            // // pangolin::GlSlProgram prog;
            // // prog.AddShader( pangolin::GlSlAnnotatedShader, my_shader );
            // // prog.Link();
            // // prog.Bind();
            // pangolin::RenderVbo(glxyz, GL_POINTS);


            // glEnableClientState(GL_VERTEX_ARRAY);
            // glEnableClientState(GL_COLOR_ARRAY);

            // glVertexPointer(3, GL_FLOAT, 0, vertices);
            // glColorPointer(3, GL_FLOAT, 0, colors);

            glDrawArrays(GL_POINTS, 0, pointCount);

            // glDisableClientState(GL_VERTEX_ARRAY); 
            // glDisableClientState(GL_COLOR_ARRAY);


            glDeleteBuffers(1,&vbo);
            // glDisableVertexAttribArray(0);
            // glDisableVertexAttribArray(1);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glUseProgram(0);
            // glPopAttrib();

           
            pangolin::FinishFrame();
            // Clear screen and activate view to render into
    }
    close(sock_fd);
}








// void showPointCloud(const vector<Vector4d, Eigen::aligned_allocator<Vector4d>> &pointcloud);

// void showPointCloud(const vector<Vector4d, Eigen::aligned_allocator<Vector4d>> &pointcloud)
// {
//     if (pointcloud.empty()){
//         cerr << "pointcloud is empty ";
//         return;
//     }
//     pangolin::CreateWindowAndBind("Main",1280,960);
//     glEnable(GL_DEPTH_TEST);
//     glEnable(GL_BLEND);

//     // Define Projection and initial ModelView matrix
//     pangolin::OpenGlRenderState s_cam(
//         pangolin::ProjectionMatrix(1280,960,420,420,320,240,0.2,10000),
//         pangolin::ModelViewLookAt(-2,2,-2, 0,0,0, pangolin::AxisY)
//     );

//     // Create Interactive View in window
//     pangolin::Handler3D handler(s_cam);
//     pangolin::View& d_cam = pangolin::CreateDisplay()
//             .SetBounds(0.0, 1.0, 0.0, 1.0, -1280.0f/960.0f)
//             .SetHandler(&handler);

//     while( !pangolin::ShouldQuit() )
//     {
//         // Clear screen and activate view to render into
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//         d_cam.Activate(s_cam);

//         glPointSize(2);
//         glBegin(GL_POINTS);

//         for (auto &p: pointcloud){
//             glColor3f(1.0,0.0,1.0);
//             glVertex3d(p[0],p[1],p[2]);
//         }
//         glEnd();

//         // Render OpenGL Cube
//         // pangolin::glDrawColouredCube();

//         // Swap frames and Process Events
//         pangolin::FinishFrame();
//     }

//     return;
// }

// int main(int argc , char *argv[]){

//     vector<Vector4d, Eigen::aligned_allocator<Vector4d>> pointCloud;
    
//     cout << "Server:\n";
//     int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
//     if(sock_fd < 0) {
//         perror("socket 创建失败");
//         return 0;
//     }
//     cout << "socket 创建成功!\n";
//     // 绑定 ip port
//     struct sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8000);
//     // inet_pton(AF_INET, "192.168.0.111", &addr.sin_addr.s_addr);
//     addr.sin_addr.s_addr = inet_addr("192.168.3.8"); //INADDR_ANY 通配地址，即本机所有 ip 都绑定上。 INADDR_ANY 转换过来就是0.0.0.0
//     int res = bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
//     if(res < 0) {
//         perror("绑定失败");
//         close(sock_fd);
//         return 0;
//     }
//     cout << "socket 绑定（命名）成功！\n";
//     struct sockaddr_in client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     int loop = 0;

//     while(1)
//     {
//         float read_buf[12001] = {0.0};
//         recvfrom(sock_fd, read_buf, sizeof(read_buf), 0, NULL, NULL);

//         int data_i = 0;
//         for(int i = 0; i < 2250; i ++)
//         {
//             Vector4d point (0, 0, 0, 1/255.0);
//             point[0] = read_buf[4 * data_i + 0];
//             point[1] = read_buf[4 * data_i + 1];
//             point[2] = read_buf[4 * data_i + 2];
//             point[3] = read_buf[4 * data_i + 3];
//             pointCloud.push_back(point);
//             data_i ++;
//         }
//         // for (auto &p: pointCloud){
//         //     printf("%f,%f,%f,%f\n",p[0],p[1],p[2],p[3]);
//         // }
//         if(loop == 200)
//         {
//             break;
//         }
//         loop++;
//     }
//     showPointCloud(pointCloud);
//     cout << "close sock_fd";

//     close(sock_fd);
//     return 0;
// }