% Check that we have an unconstrained state 
function checkUnconstrainedStateT(name,value)

    % Check for the appropriate fields 
    if ~(checkFields({ ...
        'eps_grad', ...
        'eps_dx', ...
        'stored_history', ...
        'history_reset', ...
        'iter', ...
        'iter_max', ...
        'opt_stop', ...
        'krylov_iter', ...
        'krylov_iter_max', ...
        'krylov_iter_total', ...
        'krylov_orthog_max', ...
        'krylov_stop', ...
        'krylov_rel_err', ...
        'eps_krylov', ...
        'krylov_solver', ...
        'algorithm_class', ...
        'PH_type', ...
        'H_type', ...
        'norm_gradtyp', ...
        'norm_dxtyp', ...
        'x', ...
        'grad', ...
        'dx', ...
        'x_old', ...
        'grad_old', ...
        'dx_old', ...
        'oldY', ...
        'oldS', ...
        'f_x', ...
        'f_xpdx', ...
        'msg_level', ...
        'delta', ...
        'eta1', ...
        'eta2', ...
        'ared', ...
        'pred', ...
        'rejected_trustregion', ...
        'alpha0', ...
        'alpha', ...
        'c1', ...
        'linesearch_iter', ...
        'linesearch_iter_max', ...
        'linesearch_iter_total', ...
        'eps_ls', ...
        'dir', ...
        'kind', ...
        'f_diag', ...
        'dscheme'}, ...
        value))
        error(sprintf( ...
            'The %s argument must have type Unconstrained.State.t.',name));
    end
end
