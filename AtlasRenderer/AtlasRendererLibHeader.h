#pragma once

# ifdef ATLAS_RENDERER_EXPORT
#   define ATLAS_RENDERER  __declspec( dllexport )
# else
#   define ATLAS_RENDERER __declspec( dllimport )
# endif