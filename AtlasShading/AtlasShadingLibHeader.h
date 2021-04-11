#pragma once

# ifdef ATLAS_SH_EXPORT
#   define ATLAS_SH  __declspec( dllexport )
# else
#   define ATLAS_SH __declspec( dllimport )
# endif