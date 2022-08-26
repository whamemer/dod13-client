#include "../particleman/particleman/src/public/particleman.h"

class CBaseDoDParticle : public CBaseParticle
{
public:
    void Think( float time );
    void Draw( void );
    void Animate( float time );
    void AnimateAndDie( float time );
    void Expand( float time );
    void Contract( float time );
    void Fade( float time );
    void Spin( float time );
    void CalculateVelocity( float time );
    void CheckCollision( float time );
    void Touch( Vector *p_pos, Vector *p_normal, int index );
    void Die( void );
    void Force( void );
    void InitializeSprite( Vector *p_org, Vector *p_normal, model_s *sprite, float size, float brightness );
};

class CDoDParticle : public CBaseDoDParticle
{
public:
    void SetGlobalWind( float *vecWind );
    void AddGlobalWind( void );
    void Think( float time );
    void Force( void );
    void Die( void );
    void Touch( Vector *p_pos, Vector *p_normal, int index );

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
    CDoDRocketTrail Create( Vector *p_org, Vector *p_normal, model_s *sprite, float size, float brightness, const char *classname );

private:
    bool m_bRocketTrail;
};

class CDoDDirtExploDust : public CBaseDoDParticle
{
public:
    void Think( float time );
    CDoDDirtExploDust Create( Vector *p_org, Vector *p_normal, model_s *sprite, float size, float brightness, const char *classname );

private:
    bool m_bFire;
    float m_flActivateTime;
};

class CDoDSnowFlake : public CDoDParticle
{
public:
    void Think( float time );
    void Touch( Vector *p_pos, Vector *p_normal, int index );
    CDoDSnowFlake Create( Vector *p_org, Vector *p_normal, model_s *sprite, float size, float brightness, const char *classname );

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