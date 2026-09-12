// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include "phytium_npu.h"
#include "phytium_npu_uapi.h"
#include "phytium_npu_pd2408_reg.h"

static void wake(struct phytium_npu_dev *npu)
{
	queue_work(npu->stream_wq, &npu->stream_work);
}

static int phytium_npu_check_stream_buf(struct phytium_npu_session *sess,
					struct npu_user_submit_stream *nustream)
{
	int i;
	struct npu_excute_stream *nes = &nustream->estream;

	for (i = 0; i < nes->all; i++) {
		if (!phytium_npu_find_mmu_ctx_map(sess, nes->fd[i])) {
			pr_err("Not find input dma buf fd :%d from usr strem!", nes->fd[i]);
			return -EFAULT;
		}
		pr_debug("find buffer fd %d", nes->fd[i]);
	}

	if (!phytium_npu_find_mmu_ctx_map(sess, nustream->stream_fd)) {
		pr_err("Not find stream dma buf fd :%d from usr strem!", nustream->stream_fd);
		return -EFAULT;
	}

	pr_debug("find stream fd %#x ,stream id:%#x", nustream->stream_fd, nes->sid);
	return 0;
}

static struct phytium_npu_stream *
phytium_npu_find_session_stream(struct phytium_npu_session *sess, int stream_id)
{
	struct phytium_npu_stream *nesstream, *tmp = NULL;

	WARN_ON(!sess);

	list_for_each_entry_safe(nesstream, tmp, &sess->stream_list, stream_list_entry) {
		if (nesstream->nustream.estream.sid == stream_id)
			break;
	}

	return nesstream;
}

static struct npu_user_stream_rsp *
phytium_npu_find_session_stream_rsp(struct phytium_npu_session *sess, int stream_id)
{
	struct npu_user_stream_rsp *nustream_rsp, *tmp = NULL, *rsp = NULL;

	WARN_ON(!sess);

	list_for_each_entry_safe(nustream_rsp, tmp, &sess->response_list, stream_rsp_list_entry) {
		if (nustream_rsp->ursp.sid == stream_id) {
			rsp = nustream_rsp;
			break;
		}
	}

	return rsp;
}

int phytium_npu_rollback_stream(struct phytium_npu_dev *npu)
{
	int processing = 0;
	struct phytium_npu_stream *stream;

	if (npu->activated_stream) {
		stream = npu->activated_stream;
		stream->is_rollback = TRUE;
		stream->stream_status = NPU_STREAM_NONE;
		stream->infer_status = NPU_STREAM_INFER_DONE;
		processing = TRUE;
		npu->activated_stream = NULL;
	}

	if (npu->queued_stream) {
		stream = npu->queued_stream;
		stream->is_rollback = TRUE;
		stream->stream_status = NPU_STREAM_NONE;
		stream->infer_status = NPU_STREAM_INFER_DONE;
		npu->queued_stream = NULL;
	}

	pr_debug("%s: processing %d", __func__, processing);
	return processing;
}

int phytium_npu_update_activated_stream_4_err_mmu(struct phytium_npu_dev *npu)
{
	int processing = 0;

	if (npu->activated_stream) {
		processing = TRUE;
		npu->activated_stream = NULL;
	}
	return processing;
}

int phytium_npu_try_activate_stream(struct phytium_npu_dev *npu,
				    struct phytium_npu_session *sess,
							struct phytium_npu_stream *stream)
{
	if (!npu->is_cache_stream_on) {
		if (npu->activated_stream)
			return NEED_SCHEDULE;
		goto activate_stream;
	}
	if (npu->activated_stream && npu->queued_stream) {
		pr_info("activated stream and queued stream is full");
		return NEED_SCHEDULE;
	} else if (!npu->activated_stream && npu->queued_stream) {
		pr_info("impossible here ,queued stream not NULL");
	} else if (npu->activated_stream && !npu->queued_stream) {
		return NEED_QUEUE_STREAM;
	}
	//else activate and queued stream is both NULL;
activate_stream:
	npu->activated_stream = stream;
	stream->stream_status = NPU_STREAM_IN_HW;
	stream->infer_status = NPU_STREAM_INFER_WORK;

	return phytium_npu_submit_stream(npu, sess, stream);
}

static int phytium_npu_try_queued_stream(struct phytium_npu_dev *npu,
					 struct phytium_npu_session *sess,
								struct phytium_npu_stream *stream)
{
	if (!npu->is_cache_stream_on)
		return 1;

	if (npu->activated_stream && !npu->queued_stream) {
		stream->stream_status = NPU_STREAM_NONE;
		stream->infer_status = NPU_STREAM_INFER_WAIT;
		npu->queued_stream = stream;
		pr_debug("stream had queued");
		return phytium_npu_prepare_hw_4_queued_stream(npu, sess, stream);
	}
	return 1;
}

int phytium_npu_try_excute_queued_stream(struct phytium_npu_dev *npu)
{
	struct phytium_npu_stream *stream = npu->queued_stream;

	if (!npu->is_cache_stream_on) {
		npu->activated_stream = NULL;
		pr_debug("%s, activate stream NULL", __func__);
		return 1;
	}

	if (npu->activated_stream) {
		npu->activated_stream = NULL;
	}

	if (stream) {
		npu->activated_stream = stream;
		stream->stream_status = NPU_STREAM_BUFF_IN_HW;
		stream->infer_status = NPU_STREAM_INFER_WORK;
		npu->queued_stream = NULL;
		pr_debug("%s NPU will excut queued stream", __func__);
		phytium_npu_debug_set_hw_register(npu, npu->activated_stream->session);
		phytium_npu_config_event(npu, NPU_ALL_EVENT, TRUE); /* enable all event */
		phytium_npu_config_start_inference(npu, npu->activated_stream->session,
						   npu->activated_stream);
		return 0;
	}

	return 1;
}

int phytium_npu_delete_stream(struct phytium_npu_dev *npu, struct phytium_npu_session *sess,
			      struct npu_delete_stream *stream)
{
	struct phytium_npu_stream *nesstream;
	struct npu_user_stream_rsp *nustream_rsp, *new_rsp;
	int stream_id = stream->stream_id & stream->stream_id_mask;

	nesstream = phytium_npu_find_session_stream(sess, stream_id);
	nustream_rsp = phytium_npu_find_session_stream_rsp(sess, stream_id);

	if (stream->is_res)	{
		new_rsp = kzalloc(sizeof(*new_rsp), GFP_KERNEL);
		if (!new_rsp)
			return -ENOMEM;

		INIT_LIST_HEAD(&new_rsp->stream_rsp_list_entry);
		new_rsp->ursp.sid = stream_id;
		new_rsp->ursp.err_no = NPU_RSP_OK;
		new_rsp->ursp.rsp_err_flags = 0;
		new_rsp->ursp.session_id = sess->id;
		new_rsp->rsp_size = MAX_NPU_USER_RSP_SIZE;
		new_rsp->session = NULL;
		list_add_tail(&new_rsp->stream_rsp_list_entry, &sess->response_list);
		wake_up(&sess->response_wq);
		pr_debug("delete stream will response a new msg,%s", __func__);
	}

	if (nesstream) {
		list_del(&nesstream->stream_list_entry);
		if (nesstream->rsp)	{
			if (nustream_rsp)
				list_del(&nustream_rsp->stream_rsp_list_entry);
			kfree(nesstream->rsp);
		}
		kfree(nesstream);
		nesstream = NULL;
		pr_debug("delete the stream :%d,%s", stream_id, __func__);
	}

	return 0;
}

static int phytium_npu_add_stream(struct phytium_npu_dev *npu,
				  struct phytium_npu_session *sess,
							struct phytium_npu_stream *nstream)
{
	int ret;
	struct npu_excute_stream *nesstream;

	nesstream = &nstream->nustream.estream;
	INIT_LIST_HEAD(&nstream->stream_list_entry);

	if (nesstream->stype != NPU_STREAM_SUBMIT) {
		pr_err("Stream type %d is not support !", nesstream->stype);
		return -EFAULT;
	}

	ret = phytium_npu_check_stream_buf(sess, &nstream->nustream);
	if (ret) {
		pr_err("Not find stream dma buf fd :%d from usr strem!",
		       nstream->nustream.stream_fd);
		return -EFAULT;
	}

	list_add_tail(&nstream->stream_list_entry, &sess->stream_list);
	pr_debug("add new stream to session stream_list");
	ret = phytium_npu_try_activate_stream(npu, sess, nstream);
	//TODO start a new thread to process the stream when activate the stream failed
	if (ret == NEED_SCHEDULE) {
		phytium_npu_schedule_stream_queues(npu, TRUE);
	} else if (ret == NEED_QUEUE_STREAM) {
		ret = phytium_npu_try_queued_stream(npu, sess, nstream);
		if (!ret)
			pr_debug("%s Success for queued a new stream", __func__);
		else
			pr_debug("Not Success for activating a new stream\n");
	}

	return 0;
}

static void debug_stream_info(struct phytium_npu_session *sess,
			      struct npu_user_submit_stream *stream,
							size_t size)
{
	int i = 0;

	pr_debug("stream fd:%d, size:%d, offset:%x", stream->stream_fd,
		 stream->stream_size, stream->stream_off);
	for (i = 0; i < 16; i++) {
		pr_debug("i:%d, fd:%d s:%d off:%x", i, stream->estream.fd[i],
			 stream->bufsizes[i], stream->bufoffsets[i]);
	}

	pr_debug("sess:%p SESSIONID:%d ,stream id:%x num seg:%d, stype:%d",
		 sess, sess->id, stream->estream.sid, stream->num, stream->estream.stype);
}

int phytium_npu_write_stream(struct phytium_npu_dev *npu,
			     struct phytium_npu_session *sess,
							const char *stream_buf, size_t size)
{
	struct phytium_npu_stream *nstream;
	int ret;

	nstream = kzalloc(sizeof(*nstream), GFP_KERNEL);
	if (!nstream)
		return -EINVAL;

	nstream->session = sess;
	nstream->stream_status = NPU_STREAM_NONE;
	nstream->infer_status = NPU_STREAM_INFER_NONE;
	ret = copy_from_user(&nstream->nustream, stream_buf, size);
	if (ret) {
		dev_err(npu->dev, "%s: user stream copy failed!\n", __func__);
		ret = -EFAULT;
		goto err_copy;
	}
	ret = mutex_lock_interruptible(&npu->mutex_lock);
	debug_stream_info(sess, &nstream->nustream, size);
	if (ret)
		goto err_copy;

	phytium_npu_add_stream(npu, sess, nstream);
	mutex_unlock(&npu->mutex_lock);
	return 0;

err_copy:
	kfree(nstream);

	return ret;
}

static struct phytium_npu_stream *phytium_npu_is_working_full(struct phytium_npu_dev *npu)
{
	if (npu->is_cache_stream_on) {
		if (npu->activated_stream && npu->queued_stream)
			return NULL;

		return npu->activated_stream ? npu->queued_stream : npu->activated_stream;
	}

	return npu->activated_stream;
}

static int phytium_npu_stream_is_ready_for_inference(struct phytium_npu_stream *nstream,
						     struct npu_user_submit_stream *nustream)
{
	if (nstream->stream_status > NPU_STREAM_NONE)
		return NPU_STREAM_BUFF_IN_HW;
	if (!phytium_npu_check_stream_buf_is_ready(nstream))
		return NPU_STREAM_BUFF_NO_READY;
	return 0;
}

void do_work(struct work_struct *work)
{
	struct phytium_npu_dev *npu = container_of(work, struct phytium_npu_dev, stream_work);
	struct phytium_npu_session *curr_sess, *next_sess;
	struct phytium_npu_stream *curr_stream, *next_stream;
	struct npu_user_submit_stream *nustream;
	int ret;

	mutex_lock(&npu->mutex_lock);
	if (phytium_npu_is_working_full(npu)) {
		pr_debug("NPU is working for inference");
		mutex_unlock(&npu->mutex_lock);
		return;
	}

	do {
		list_for_each_entry_safe(curr_sess, next_sess, &npu->sched_sess_list,
					 sched_list_entry) {
			list_for_each_entry_safe(curr_stream, next_stream,
						 &curr_sess->stream_list,
						 stream_list_entry) {
				if (curr_stream->stream_status >= NPU_STREAM_IN_HW ||
				    curr_stream->infer_status)
					continue;

				nustream = &curr_stream->nustream;
				pr_debug("estream type:%#x", nustream->estream.stype);
				if (nustream->estream.stype == NPU_STREAM_SUBMIT) {
					ret = phytium_npu_stream_is_ready_for_inference(curr_stream,
											nustream);
					pr_debug("strem is ready:%d", ret);
					if (ret)/* current stream is not ready */
						continue;
					ret = phytium_npu_try_activate_stream(npu,
									      curr_sess,
									      curr_stream);
					if (!ret) {
						phytium_npu_change_schedule_session(curr_sess,
										    npu, TRUE);
						pr_debug("Change schedule session locate");
						goto out_schedule;
					} else if (ret == NEED_QUEUE_STREAM) {
						ret = phytium_npu_try_queued_stream(npu,
										    curr_sess,
										curr_stream);
					}

					if (!ret)
						pr_debug("%s Success for queued a new stream",
							 __func__);
				}
			}
		}
out_schedule:;
	} while (0);
	mutex_unlock(&npu->mutex_lock);
}

void phytium_npu_schedule_stream_queues(struct phytium_npu_dev *npu, bool asynchronous)
{
	if (asynchronous) {
		pr_debug("asynchronous schedule do work");
		wake(npu);
	} else {
		pr_debug("synchronous schedule do work");
		mutex_unlock(&npu->mutex_lock);
		do_work(&npu->stream_work);
		mutex_lock(&npu->mutex_lock);
	}
}
