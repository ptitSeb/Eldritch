#ifndef GL2SHADERPROGRAM_H
#define GL2SHADERPROGRAM_H

#include "ishaderprogram.h"
#include "gl2.h"
#include "map.h"

class GL2ShaderProgram : public IShaderProgram
{
public:
	GL2ShaderProgram();
	virtual ~GL2ShaderProgram();

	virtual void			Initialize( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration );
	virtual void*			GetHandle() { return &m_ShaderProgram; }
	virtual uint			GetVertexSignature() const { return m_VertexSignature; }
	virtual IVertexShader*	GetVertexShader() const { return m_VertexShader; }
	virtual IPixelShader*	GetPixelShader() const { return m_PixelShader; }
	virtual bool			GetVertexShaderRegister( const HashedString& Parameter, uint& Register ) const;
	virtual bool			GetPixelShaderRegister( const HashedString& Parameter, uint& Register ) const;
	virtual bool			IsCached( uint Register, int Size, const void* Value );

protected:
	void					BindAttributes( IVertexDeclaration* const pVertexDeclaration ) const;
	void					BuildUniformTable();
	void					SetSamplerUniforms();
	bool					GetRegister( const HashedString& Parameter, uint& Register ) const;

	uint			m_VertexSignature;
	IVertexShader*	m_VertexShader;
	IPixelShader*	m_PixelShader;

	GLuint			m_ShaderProgram;

	Map<HashedString, uint>	m_UniformTable;
	GLuint			m_MaxCache;
	void**			m_UniformCache;
	int*			m_UniformSize;
};

#endif // GL2SHADERPROGRAM_H
