
from imvu import libzero
from imvu import avatarwindow

import cal3d

class materialTestData:

    material_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "</MATERIAL>"


    material_no_textures =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"0\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "</MATERIAL>"

    material_without_opacity_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "</MATERIAL>"

    material_with_unsupported_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "<MAP TYPE=\"Unsupported\">%s</MAP>"\
        "</MATERIAL>"

    

    basecoat_material_template =\
        "<HEADER MAGIC=\"XRF\" VERSION=\"919\" />"\
        "<MATERIAL NUMMAPS=\"2\">"\
        "<AMBIENT>149 149 149 0</AMBIENT>"\
        "<DIFFUSE>255 255 255 255</DIFFUSE>"\
        "<SPECULAR>229 229 229 0</SPECULAR>"\
        "<SHININESS>0</SHININESS>"\
        "<MAP TYPE=\"Diffuse Color\">%s</MAP>"\
        "<MAP TYPE=\"Opacity\">%s</MAP>"\
        "</MATERIAL>"

   

    def __init__(self, 
        diffuseMapName="foo_diffuse.png", 
        opacityMapName="foo_opacity.png"):
        self.stringToTextureMap = {}
        self.diffuseMapName = diffuseMapName
        self.opacityMapName = opacityMapName
        self.basecoatDiffuseMapName = "basecoat_diffuse.jpg"
        self.basecoatOpacityMapName = "basecoat_opacity.jpg"        
        self.material = materialTestData.material_template % (self.diffuseMapName, self.opacityMapName)        
        self.material_without_opacity = materialTestData.material_without_opacity_template % self.diffuseMapName
        self.material_with_unsupported = materialTestData.material_with_unsupported_template % (self.diffuseMapName, self.opacityMapName, "unsupported.jpg")
        self.basecoat_material = materialTestData.basecoat_material_template % (self.basecoatDiffuseMapName, self.basecoatOpacityMapName) 

    def setUp(self):  
        self.stringToTextureMap = {}   

    def defineTexture(self, name, modelTextureData):
        self.stringToTextureMap[name] = modelTextureData

    def defineTextureColor(self, name, r, g, b, size=libzero.Vec2i(1, 1)):
        rgb = chr(r) + chr(g) + chr(b)
        s = size.x * size.y * rgb
        self.stringToTextureMap[name] = avatarwindow.ModelTextureData(
            avatarwindow.AssetSource("", name),
            s,
            size.x,
            size.y,
            size.x,
            size.y,
            False)

    def getTextureData(self, name):
        return self.stringToTextureMap[name]

meshDataPrefix =\
    "<HEADER MAGIC=\"XMF\" VERSION=\"919\" />"\
    "<MESH NUMSUBMESH=\"1\">"

submeshTemplate01 =\
    "   <SUBMESH NUMVERTICES=\"3\" NUMFACES=\"1\" NUMLODSTEPS=\"0\" NUMSPRINGS=\"0\" NUMMORPHS=\"1\" NUMTEXCOORDS=\"1\" MATERIAL=\"%d\">"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"0\">"\
    "           <POS>5759.05 -1176.88 -0.00023478</POS>"\
    "           <NORM>1.27676e-008 2.40249e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.00973237</TEXCOORD>"\
    "           <INFLUENCE ID=\"0\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"2\" ID=\"1\">"\
    "           <POS>-5759.05 -1176.88 -0.000413365</POS>"\
    "           <NORM>1.55047e-008 -2.86491e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"0\">0.5</INFLUENCE>"\
    "           <INFLUENCE ID=\"1\">0.5</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"2\">"\
    "           <POS>-5759.05 -3274.86 -0.000507484</POS>"\
    "           <NORM>1.11221e-008 6.89228e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.79062 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"1\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <MORPH NAME=\"ogle\" NUMBLENDVERTS=\"1\" MORPHID=\"0\">"\
    "           <BLENDVERTEX VERTEXID=\"2\">"\
    "               <POSITION>6000 -1500 0</POSITION>"\
    "               <NORMAL>0 0.707 -0.707</NORMAL>"\
    "               <TEXCOORD>0.0 0.25</TEXCOORD>"\
    "           </BLENDVERTEX>"\
    "       </MORPH>"\
    "       <FACE VERTEXID=\"0 1 2\" />"\
    "    </SUBMESH>"

submeshTemplate02 =\
    "   <SUBMESH NUMVERTICES=\"3\" NUMFACES=\"1\" NUMLODSTEPS=\"0\" NUMSPRINGS=\"0\" NUMMORPHS=\"0\" NUMTEXCOORDS=\"1\" MATERIAL=\"0\">"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"0\">"\
    "           <POS>5759.05 -1176.88 -0.00023478</POS>"\
    "           <NORM>1.27676e-008 2.40249e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.00973237</TEXCOORD>"\
    "           <INFLUENCE ID=\"3\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"2\" ID=\"1\">"\
    "           <POS>-5759.05 -1176.88 -0.000413365</POS>"\
    "           <NORM>1.55047e-008 -2.86491e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.99311 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"9\">0.5</INFLUENCE>"\
    "           <INFLUENCE ID=\"10\">0.5</INFLUENCE>"\
    "       </VERTEX>"\
    "       <VERTEX NUMINFLUENCES=\"1\" ID=\"2\">"\
    "           <POS>-5759.05 -3274.86 -0.000507484</POS>"\
    "           <NORM>1.11221e-008 6.89228e-008 -1</NORM>"\
    "           <COLOR>0 0 0</COLOR>"\
    "           <TEXCOORD>0.79062 0.982444</TEXCOORD>"\
    "           <INFLUENCE ID=\"5\">1</INFLUENCE>"\
    "       </VERTEX>"\
    "       <FACE VERTEXID=\"0 1 2\" />"\
    "    </SUBMESH>"

meshDataSuffix =\
    "</MESH>"

def makeMeshTestData(materialIds):
    meshData = meshDataPrefix
    for materialId in materialIds:
        meshData += submeshTemplate01 % (materialId,)
    meshData += meshDataSuffix
    return meshData

def makeMeshTestDataForDualUseBones():
    return meshDataPrefix + submeshTemplate02 + meshDataSuffix

skeletonTestData = \
"""<HEADER VERSION=\"910\" MAGIC="XSF" />
    <SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="FooRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 0 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="FooChild" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="FooAdjustedRoot" NUMCHILDS="0" ID="2">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
    </BONE>
    <BONE NAME="camera.01.01.root" NUMCHILDS="0" ID="3">
        <TRANSLATION>0 0 0</TRANSLATION>
        <ROTATION>-0.70710678 0 0 0.70710678</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0.70710678 0 0 0.70710678</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="Omni01" NUMCHILDS="0" ID="1" LIGHTTYPE="1" LIGHTCOLOR="0.5 0.25 0.125">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -500</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="Spot01" NUMCHILDS="0" ID="1" LIGHTTYPE="2" LIGHTCOLOR="1.0 0.9 0.8">
        <TRANSLATION>0 -10 450</TRANSLATION>
        <ROTATION>-0.70710678 0 0 0.70710678</ROTATION>
        <LOCALTRANSLATION>0 10 -450</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="SeeThrough09" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 400</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -400</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="furniture.Wall.01" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 350</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -350</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="furniture.Ceiling.01" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 300</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -300</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="furniture.Floor.01" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 300</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -300</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="Seat01.Standing" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 300</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -300</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="Seat02.Bench" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 300</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -300</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""

skeletonWithNoCameraTestData = \
"""<HEADER VERSION=\"910\" MAGIC="XSF" />
    <SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="FooRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 0 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="FooChild" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
    <BONE NAME="FooAdjustedRoot" NUMCHILDS="0" ID="2">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
    </BONE>
</SKELETON>"""

skeletonAnimationTestData = \
"""
<HEADER VERSION="910" MAGIC="XAF" />
<ANIMATION NUMTRACKS="2" DURATION="2">
    <TRACK NUMKEYFRAMES="2" BONEID="0">
        <KEYFRAME TIME="0">
            <TRANSLATION>0 10 20</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>0 10 20</TRANSLATION>
            <ROTATION>0.564503 -0.461901 0.501256 -0.465538</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="2" BONEID="1">
        <KEYFRAME TIME="0">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>-0.0611443 -0.627387 0.771113 0.0896201</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="2" BONEID="2">
        <KEYFRAME TIME="0">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>-0.0611443 -0.627387 0.771113 0.0896201</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="2" BONEID="11">
        <KEYFRAME TIME="0">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>0.0 0.0 0.0 1.0</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="1">
            <TRANSLATION>40 50 60</TRANSLATION>
            <ROTATION>0.0 0.0 0.0 1.0</ROTATION>
        </KEYFRAME>
    </TRACK>
</ANIMATION>
"""

attachmentSkeletonTestData = \
"""<HEADER VERSION=\"910\" MAGIC="XSF" />
    <SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="AuxRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 0 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 0 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="AuxChild" NUMCHILDS="0" ID="1">
        <TRANSLATION>0 -10 500</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10 -470</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""

morphAnimationTestData = \
"""<HEADER VERSION="910" MAGIC="XPF" />
    <ANIMATION NUMTRACKS="1" DURATION="0.166667">
     <TRACK NUMKEYFRAMES="3" MORPHNAME="eyes.Blink.Clamped">
        <KEYFRAME TIME="0">
            <WEIGHT>0</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.0333333">
            <WEIGHT>0.5</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.0666667">
            <WEIGHT>1</WEIGHT>
        </KEYFRAME>
     </TRACK>
    </ANIMATION>"""


def makeTriangularStripSubmesh():
    numVertices = 10
    numTris = 8
    """

    0        2        4        6        8
    X--------X--------X--------X--------X
    |\,      |\,      |\,      |\,      |
    |  \bo   |  \b1   |  \b2   |  \,b3  |
    |    \,  |    \,  |    \,  |    \,  |
    |      \,|      \,|      \,|      \,|
    X--------X--------X--------X--------X
    1        3        5        7        9

    10 vertices numbered 0-9
    8 faces
    4 bones, b0-b3
    vertices 0,1 are weighted fully to b0
    vertices 8,9 are weighted fully to b3
    every other vertex is weighted equally to the neighboring two bones
    eg: vertex 3 is weighted to b0 and b1 with weights 0.5

    """
    #make the test mesh
    bigMesh = cal3d.CoreSubmesh(numVertices, True, numTris)
    def print_mesh(m):
        for v in m.vertices:
            print '%r, %r' % (v.position, v.normal)
        for t in m.triangles:
            print '%r %r %r' % (t.v1, t.v2, t.v3)
        for i in m.influences:
            print '%r, %r' % (i.boneId, i.weight)

    def makeVertex(smesh, upperRow, infs, xPos):
        v = cal3d.Vertex()
        influences = cal3d.InfluenceVector()
        for i in infs:
            inf = cal3d.Influence(i[0], i[1], False)
            influences.append(inf)
        inf = influences[-1]
        inf.isLast = True
        if upperRow:
            v.position = cal3d.Vector(xPos, 0.0, 0.0)
        else:
            v.position = cal3d.Vector(xPos, -1.0, 0.0)
        v.normal = cal3d.Vector(0.0, 0.0, 1.0)
        smesh.addVertex(v, 0, influences)
    makeVertex(bigMesh, upperRow=True, infs=[(0,1.0)], xPos=0.0)
    makeVertex(bigMesh, upperRow=False, infs=[(0,1.0)], xPos=0.0)
    makeVertex(bigMesh, upperRow=True, infs=[(0, 0.5),(1, 0.5)], xPos=1.0)
    makeVertex(bigMesh, upperRow=False, infs=[(0, 0.5),(1, 0.5)], xPos=1.0)
    makeVertex(bigMesh, upperRow=True, infs=[(1, 0.5),(2, 0.5)], xPos=2.0)
    makeVertex(bigMesh, upperRow=False, infs=[(1, 0.5),(2, 0.5)], xPos=2.0)
    makeVertex(bigMesh, upperRow=True, infs=[(2, 0.5),(3, 0.5)], xPos=3.0)
    makeVertex(bigMesh, upperRow=False, infs=[(2, 0.5),(3, 0.5)], xPos=3.0)
    makeVertex(bigMesh, upperRow=True, infs=[(3, 1.0)], xPos=4.0)
    makeVertex(bigMesh, upperRow=False, infs=[(3, 1.0)], xPos=4.0)
    triangles = bigMesh.triangles
    bigMesh.addTriangle(cal3d.Triangle(0, 1, 3))
    bigMesh.addTriangle(cal3d.Triangle(0, 3, 2))
    bigMesh.addTriangle(cal3d.Triangle(2, 3, 5))
    bigMesh.addTriangle(cal3d.Triangle(2, 5, 4))
    bigMesh.addTriangle(cal3d.Triangle(4, 5, 7))
    bigMesh.addTriangle(cal3d.Triangle(4, 7, 6))
    bigMesh.addTriangle(cal3d.Triangle(6, 7, 9))
    bigMesh.addTriangle(cal3d.Triangle(6, 9, 8))

    vIdx = 0
    for v in bigMesh.vertices:
        tcoord = cal3d.TextureCoordinate(v.position.x/4.0, -v.position.y)
        bigMesh.setTextureCoordinate(vIdx, tcoord)

    return bigMesh
