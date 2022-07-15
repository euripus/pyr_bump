

update_joints(float time)
	for(e : reg.view<Model, Joint>())
		cur_anim = reg.get<CurrentAnimSequence>().id
		as = mdl.animations[cur_anim]
		frame = getCurFrame(time, as)

		update_model_joints(mdl, frame)
		update_model_bbox(mdl, frame)

update_model_joints(mdl, frame)
	for(i=0; i < bone_id_to_entity.size(), i++)
		ent = bone_id_to_entity[i]
		mat = frame.mat[i]

		TransformComponent trans;
		trans.replase_local_matrix = true;
		trans.rel = mat
		reg.add<TransformComponent>(ent, mat)

// .....
update_meshes
	for(e : reg.view<Model, Joint>())
		for(msh : mdl.meshes)
			for(vtx : msh)
				for(ws : we)
					joint = getJoint(ws.jnt_indx)
					jnt_pos = reg.get<Pos>(joint)
					mat = mat * ws.w(model_inv * jnt_pos.abs)
				new_vtx = mat * vtx

Model : [pos] [material] [meshes] [root_joint] [cur_anim_sequence]
Joint : [pos] [joint_node]