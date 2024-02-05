#include "main.hpp"
#include "Shape.hpp"

#include <iostream>
#include <glm/ext.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


const float PI = 3.14159f;
const float rad = PI / 180.0f;

unsigned int VaoFromTris(std::vector<glm::vec4>& Pnt, std::vector<glm::vec3>& Nrm,
	std::vector<glm::vec2>& Tex, std::vector<glm::vec3>& Tan, std::vector<glm::ivec3>& Tri)
{
	unsigned int vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint Pbuff;
	glGenBuffers(1, &Pbuff);
	glBindBuffer(GL_ARRAY_BUFFER, Pbuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * Pnt.size(),
		&Pnt[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (Nrm.size() > 0) {
		GLuint Nbuff;
		glGenBuffers(1, &Nbuff);
		glBindBuffer(GL_ARRAY_BUFFER, Nbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Nrm.size(),
			&Nrm[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (Tex.size() > 0) {
		GLuint Tbuff;
		glGenBuffers(1, &Tbuff);
		glBindBuffer(GL_ARRAY_BUFFER, Tbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * Tex.size(),
			&Tex[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (Tan.size() > 0) {
		GLuint Dbuff;
		glGenBuffers(1, &Dbuff);
		glBindBuffer(GL_ARRAY_BUFFER, Dbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * Tan.size(),
			&Tan[0][0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GLuint Ibuff;
	glGenBuffers(1, &Ibuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * Tri.size(),
		&Tri[0][0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	return vaoID;
}

void pushquad(std::vector<glm::ivec3>& Tri, int i, int j, int k, int l)
{
	Tri.push_back(glm::ivec3(i, j, k));
	Tri.push_back(glm::ivec3(i, k, l));
}

void Shape::MakeVAO()
{
	vaoID = VaoFromTris(Pnt, Nrm, Tex, Tan, Tri);
	count = Tri.size();
}

void Shape::DrawVAO()
{
	glBindVertexArray(vaoID);
	glDrawElements(GL_TRIANGLES, 3 * count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Shape::DrawLineVAO()
{
	glBindVertexArray(vaoID);
	glDrawElements(GL_LINES, 3 * count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

Box::Box()
{
	glm::mat4 I(1.0f);

	// Six faces, each a rotation of a rectangle placed on the z axis.
	face(I);
	float r90 = PI / 2;
	face(glm::rotate(I, r90, glm::vec3(1.0f, 0.0f, 0.0f)));
	face(glm::rotate(I, -r90, glm::vec3(1.0f, 0.0f, 0.0f)));
	face(glm::rotate(I, r90, glm::vec3(0.0f, 1.0f, 0.0f)));
	face(glm::rotate(I, -r90, glm::vec3(0.0f, 1.0f, 0.0f)));
	face(glm::rotate(I, PI, glm::vec3(1.0f, 0.0f, 0.0f)));

	MakeVAO();
}

void Box::face(const glm::mat4 tr)
{
	int n = Pnt.size();

	float verts[8] = { 1.0f,1.0f, -1.0f,1.0f, -1.0f,-1.0f, 1.0f,-1.0f };
	float texcd[8] = { 1.0f,1.0f,  0.0f,1.0f,  0.0f, 0.0f, 1.0f, 0.0f };

	// Four vertices to make a single face, with its own normal and
	// texture coordinates.
	for (int i = 0; i < 8; i += 2) {
		Pnt.push_back(tr * glm::vec4(verts[i], verts[i + 1], 1.0f, 1.0f));
		Nrm.push_back(glm::vec3(tr * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
		Tex.push_back(glm::vec2(texcd[i], texcd[i + 1]));
		Tan.push_back(glm::vec3(tr * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	}

	pushquad(Tri, n, n + 1, n + 2, n + 3);
}

Sphere::Sphere(const int n)
{
	for (int i = 0; i <= n * 2; i++)
	{
		float s = i * 2.0f * PI / float(n * 2);

		for (int j = 0; j <= n; j++)
		{
			float t = j * PI / float(n);
			float x = cos(s) * sin(t);
			float y = sin(s) * sin(t);
			float z = cos(t);

			Pnt.push_back(glm::vec4(x, y, z, 1.0f));
			Nrm.push_back(glm::vec3(x, y, z));
			Tex.push_back(glm::vec2(s / (2 * PI), t / PI));
			Tan.push_back(glm::vec3(-sin(s), cos(s), 0.0));

			if (i > 0 && j > 0)
			{
				pushquad(Tri, (i - 1) * (n + 1) + (j - 1),
					(i - 1) * (n + 1) + (j),
					(i) * (n + 1) + (j),
					(i) * (n + 1) + (j - 1));
			}
		}
	}

	MakeVAO();
}

Plane::Plane(const float r, const int n)
{
	for (int i = 0; i <= n; i++)
	{
		float s = i / float(n);

		for (int j = 0; j <= n; j++)
		{
			float t = j / float(n);
			Pnt.push_back(glm::vec4(s * 2.0 * r - r, t * 2.0 * r - r, 0.0, 1.0));
			Nrm.push_back(glm::vec3(0.0, 0.0, 1.0));
			Tex.push_back(glm::vec2(s, t));
			Tan.push_back(glm::vec3(1.0, 0.0, 0.0));

			if (i > 0 && j > 0)
			{
				pushquad(Tri, (i - 1) * (n + 1) + (j - 1),
					(i - 1) * (n + 1) + (j),
					(i) * (n + 1) + (j),
					(i) * (n + 1) + (j - 1));
			}
		}
	}

	MakeVAO();
}

Quad::Quad(const int n)
{
	float r = 1.0;

	for (int i = 0; i <= n; i++)
	{
		float s = i / float(n);
		for (int j = 0; j <= n; j++)
		{
			float t = j / float(n);
			Pnt.push_back(glm::vec4(s * 2.0 * r - r, t * 2.0 * r - r, 0.0, 1.0));
			Nrm.push_back(glm::vec3(0.0, 0.0, 1.0));
			Tex.push_back(glm::vec2(s, t));
			Tan.push_back(glm::vec3(1.0, 0.0, 0.0));
			if (i > 0 && j > 0)
			{
				pushquad(Tri,
					(i - 1) * (n + 1) + (j - 1),
					(i - 1) * (n + 1) + (j),
					(i) * (n + 1) + (j),
					(i) * (n + 1) + (j - 1));
			}
		}
	}

	MakeVAO();
}


void CreateMesh(aiMesh* mesh, const aiScene* scene,
	std::vector<glm::vec4>& Pnt, std::vector<glm::vec3>& Nrm, std::vector<glm::vec2>& Tex, std::vector<glm::vec3>& Tan, std::vector<glm::ivec3>& Tri)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Pnt.push_back({ mesh->mVertices[i].x , mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f });
		Nrm.push_back({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });

		if (mesh->mTextureCoords[0])
		{
			Tex.push_back({ mesh->mTextureCoords[0][i].x , mesh->mTextureCoords[0][i].y });
		}

		if (mesh->mTangents)
		{
			Tan.push_back({ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
		}
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		Tri.push_back({ face.mIndices[0], face.mIndices[1], face.mIndices[2] });
	}
}

void CreateNode(aiNode* node, const aiScene* scene,
	std::vector<glm::vec4>& Pnt, std::vector<glm::vec3>& Nrm, std::vector<glm::vec2>& Tex, std::vector<glm::vec3>& Tan, std::vector<glm::ivec3>& Tri)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		CreateMesh(mesh, scene, Pnt, Nrm, Tex, Tan, Tri);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		CreateNode(node->mChildren[i], scene, Pnt, Nrm, Tex, Tan, Tri);
}

Obj::Obj(const std::string name)
{
	Assimp::Importer importer;
	const std::string path = prefix + name;
	auto flag = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords
		| aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality;

	const aiScene* scene = importer.ReadFile(path.c_str(), flag);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	aiNode* node = scene->mRootNode;
	CreateNode(node, scene, Pnt, Nrm, Tex, Tan, Tri);

	MakeVAO();
}