[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 2
  []

  [left]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = left
    block_id = 2
    bottom_left = '0 1 0'
    top_right = '1 2 0'
  []
[]

[Postprocessors]
  [block1_qps]
    type = NumElemQPs
    block = 1
  []
  [block2_qps]
    type = NumElemQPs
    block = 2
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  [Quadrature]
    custom_blocks = '1 2'
    custom_orders = 'first second'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  csv = true
[]
