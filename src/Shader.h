//
// Created by Fedor Artemenkov on 05.07.2024.
//

#pragma once


class Shader
{
public:
    Shader();

    Shader(const Shader&) = delete;             // no copy allowed
    Shader& operator =(const Shader&) = delete; // no copy allowed

    Shader(Shader&& other) noexcept;            // move allowed
    Shader& operator=(Shader&& other) noexcept; // move allowed

    ~Shader();

public:
    void init(const char* filename);
    void init(const char* vert_src, const char* frag_src);

    void bind() const;
    void unbind() const;

    void setUniformMatrix(const float* data, const char* name) const;
    void setUniformVector4(const float* data, const char* name) const;

//private:
    int program;
};
