#include "../particleman/particleman/src/public/particleman.h"
#include "vector.h"

class CBaseDoDParticle : public CBaseParticle
{
public:
    virtual void Think( float time ) { g_pParticleMan->CoreThink( this, time ); }
    virtual void Draw( void ) { g_pParticleMan->CoreDraw( this ); }
    virtual void Animate( float time ) { g_pParticleMan->CoreAnimate( this, time ); }
    virtual void AnimateAndDie( float time ) { g_pParticleMan->CoreAnimateAndDie( this, time ); }
    virtual void Expand( float time ) { g_pParticleMan->CoreExpand( this, time ); }
    virtual void Contract( float time ) { g_pParticleMan->CoreContract( this, time ); }
    virtual void Fade( float time ) { g_pParticleMan->CoreFade( this, time ); }
    virtual void Spin( float time ) { g_pParticleMan->CoreSpin( this, time ); }
    virtual void CalculateVelocity( float time ) { g_pParticleMan->CoreCalculateVelocity( this, time ); }
    virtual void CheckCollision( float time ) { g_pParticleMan->CoreCheckCollision( this, time ); }
    virtual void Touch( Vector pos, Vector normal, int index ) { g_pParticleMan->CoreTouch( this, pos, normal, index ); }
    virtual void Die( void ) { g_pParticleMan->CoreDie( this ); }
    virtual void Force( void ) { g_pParticleMan->CoreForce( this ); }

    virtual void InitializeSprite( Vector org, Vector normal, model_s *sprite, float size, float brightness )
    {
	    g_pParticleMan->CoreInitializeSprite( this, org, normal, sprite, size, brightness );
	}
};

class CDoDParticle : public CBaseDoDParticle
{
public:
    void SetGlobalWind( float *vecWind );
    void AddGlobalWind( void );
    void Think( float time );
    void Force( void );
    void Die( void );
    void Touch( Vector pos, Vector normal, int index );
    CDoDParticle *CDoDParticle::Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname, bool bDistCull );

private:
    bool m_bInsideSmoke;
    bool m_bSpawnInside;
    bool m_bDampMod;
    bool m_bAffectedByForce;

    int m_iPFlags;

    Vector m_vRingOrigin;
    static Vector m_vGlobalWind;
};

class TriangleWallPuff : public CBaseParticle
{
public:
    void Think( float time );

private:
    float m_flActivateTime;
};

class CDoDRocketTrail : public CBaseDoDParticle
{
public:
    void Think( float time );
    CDoDRocketTrail Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

private:
    bool m_bRocketTrail;
};

class CDoDDirtExploDust : public CBaseDoDParticle
{
public:
    void Think( float time );
    CDoDDirtExploDust Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

private:
    bool m_bFire;
    float m_flActivateTime;
};

class CDoDSnowFlake : public CDoDParticle
{
public:
    void Think( float time );
    void Touch( Vector pos, Vector normal, int index );
    CDoDSnowFlake Create( Vector org, Vector normal, model_s *sprite, float size, float brightness, const char *classname );

private:
    bool m_bSpiral;
    bool m_bTouched;

    float m_flOldTime;
    float m_flFadeOutTime;
};

class CDoDRainDrop : public CDoDParticle
{
public:
    void Think( float time );
};