#ifndef _GenTypeDefs_H
#define _GenTypeDefs_H

typedef enum
{ charm, beauty, charm_unforced, beauty_unforced, jpsi, jpsi_chi }
Process_t;

typedef enum
{ semielectronic, dielectron, semimuonic, dimuon,
  b_jpsi_dimuon, b_jpsi_dielectron, 
  b_psip_dimuon, b_psip_dielectron }
Decay_t;

typedef enum
{
    DO_Set_1=1006,
    GRV_LO=5005,
    GRV_HO=5006,
    MRS_D_minus=3031,
    MRS_D0=3030,
    MRS_G=3041,
    CTEQ_2pM=4024,
    CTEQ_4M=4034
}
StrucFunc_t;

#endif


